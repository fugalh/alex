#include "iax.h"
#include "codec.h"

#define BUFSIZE (1024)

void *protocol_thread_func(void *arg)
{
    IAX2Protocol *iax = (IAX2Protocol*)arg;
    iax->event_loop();
    return 0;
}

IAX2Protocol::IAX2Protocol(Audio *audio, int format) : audio(audio)
{
    port = iax_init(IAX_DEFAULT_PORTNO);
    session = iax_session_new();
    gsm_handle = gsm_create();
    set_codec(format);

    pthread_create(&thread, 0, protocol_thread_func, this);
}

IAX2Protocol::~IAX2Protocol()
{
    pthread_cancel(thread);
    iax_destroy(session);
    gsm_destroy(gsm_handle);
    if (codec) delete codec;
}


int IAX2Protocol::call(char* cidnum, char* cidname, char* ich)
{
    return iax_call(session, cidnum, cidname, ich, 
	    NULL, 1, AST_FORMAT_GSM, AST_FORMAT_GSM);
    audio->off_hook = 1;
    return ret;
}

int IAX2Protocol::hangup(char* byemsg)
{
    int ret = iax_hangup(session, byemsg);
    audio->off_hook = 0;
    return ret;
}

void IAX2Protocol::event_loop()
{
    while(1)
    {
	struct iax_event *ev = iax_get_event(1);
        if (ev)
        {
	    int ret;
	    gsm_signal src[160];
	    gsm_frame dst;

	    //read
	    jack_ringbuffer_read(audio->input_rb, (char*)src, sizeof(src));

            //encode
	    gsm_encode(gsm_handle, src, dst);

            //send
	    ret = iax_send_voice(session, AST_FORMAT_GSM, (char*)dst, 33, 160);
        }
        if (jack_ringbuffer_read_space(iax_event_rb) > 0)
        {
            struct iax_event *ev;
            jack_ringbuffer_read(iax_event_rb, (char*)&ev, sizeof(ev));

	    switch (ev->etype)
	    {
		case IAX_EVENT_ACCEPT:
		    printf("Call accepted.\n");
		    break;
		case IAX_EVENT_REJECT:
		    printf("Call rejected.\n");
		    return;
		    break;
		case IAX_EVENT_ANSWER:
		    printf("Call answered.\n");
		    break;
		case IAX_EVENT_VOICE:
		    handle_voice(ev);
		    break;
		case IAX_EVENT_PONG:
		    printf("Pong!\n");
		    break;
		case IAX_EVENT_HANGUP:
		    printf("Hung up.\n");
		    exit(0);
		    break;
		default:
		    fprintf(stderr,"Unhandled packet of type %d.\n",ev->etype);
	    }

            iax_event_free(ev);
        }

        short src[BUFSIZE];
        int srclen = audio->read(src, BUFSIZE);
        if (srclen > 0)
        {
            char dst[BUFSIZE];
            int dstlen = BUFSIZE;
            codec->encode(src, &srclen, dst, &dstlen);
	    iax_send_voice(session, codec_format, dst, dstlen, srclen);
        }
    }
}

int IAX2Protocol::handle_voice(struct iax_event *ev)
{
    if (ev->subclass == codec_format)
    {
        char *src = (char*)ev->data;
        int srclen = ev->datalen;
        short dst[BUFSIZE];
        int dstlen = BUFSIZE;
        codec->decode(src, &srclen, dst, &dstlen);
        audio->write(dst,dstlen);
    }
    else
        fprintf(stderr,"Unknown voice format %d.\n", ev->subclass);
        return 1;
#if 0
    switch (ev->subclass)
    {
	case AST_FORMAT_GSM:
	{
	    if (ev->datalen % 33)
	    {
		fprintf(stderr,"Bad GSM data (len != 0 mod 33).\n");
		return 1;
	    }

	    // convert from gsm (ev->data) to 8000hz short (audio->output_rb)
	    gsm_byte *src;
	    gsm_signal dst[160];
	    for (int i=0; i < ev->datalen/33; i += 33)
	    {
		src = (gsm_byte*)ev->data + i;
		gsm_decode(gsm_handle, src, dst);
		int ret = jack_ringbuffer_write(audio->output_rb, (char*)dst, 
			sizeof(dst));
		if (ret < sizeof(dst))
		    fprintf(stderr,"overrun in handle_voice(): "
			    "%d bytes. (%d)\n", sizeof(dst) - ret, 
			    jack_ringbuffer_write_space(audio->output_rb));
	    }
	    break;
	}
	default:
	    fprintf(stderr,"Unknown voice format %d.\n", ev->subclass);
	    return 1;
    }
#endif
    return 0;
}

int IAX2Protocol::dtmf(const char c)
{
    return iax_send_dtmf(session, c);
}

void IAX2Protocol::set_codec(int format)
{
    switch(format)
    {
        case AST_FORMAT_GSM:
            codec_format = format;
            codec = new GSM();
            break;
        default:
            fprintf(stderr,"Unrecognized format in set_codec(%d)\n",format);
            codec = 0;
    }
}
