#include "iax.h"
#include "codec.h"
#include "gsm.h"

#define BUFSIZE (1024)

void *protocol_thread_func(void *arg)
{
    IAX *iax = (IAX*)arg;
    iax->event_loop();
    return 0;
}

IAX::IAX(Audio *a)
{
    audio = a;
    port = iax_init(IAX_DEFAULT_PORTNO);
    session = iax_session_new();

    pthread_create(&thread, 0, protocol_thread_func, this);
}

IAX::~IAX()
{
    pthread_cancel(thread);
    iax_destroy(session);
}


int IAX::call(char* cidnum, char* cidname, char* ich)
{
    int cap = 0;
    for (unsigned int i=0; i<codecs.size(); i++)
        cap &= codecs[i];

    if (codecs.size() < 1)
        return -1; // XXX is this a good return value?

    return iax_call(session, cidnum, cidname, ich, 
	    NULL, 1, codecs[0], cap);
}

int IAX::hangup(char* byemsg)
{
    int ret = iax_hangup(session, byemsg);
    audio->off_hook = 0;
    return ret;
}

void IAX::event_loop()
{
    while(1)
    {
	struct iax_event *ev = iax_get_event(1);
        if (ev)
        {
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
		    audio->off_hook = 1;
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
            get_codec(codecs[0])->encode(src, &srclen, dst, &dstlen);
	    iax_send_voice(session, codecs[0], dst, dstlen, srclen);
        }
    }
}

int IAX::handle_voice(struct iax_event *ev)
{
    Codec *codec = get_codec(ev->subclass);
    if (codec)
    {
        char *src = (char*)ev->data;
        int srclen = ev->datalen;
        short dst[BUFSIZE];
        int dstlen = BUFSIZE;
        codec->decode(src, &srclen, dst, &dstlen);
        audio->write(dst,dstlen);
    }
    else
    {
        fprintf(stderr,"Unknown voice format %d.\n", ev->subclass);
        return 1;
    }
    return 0;
}

int IAX::dtmf(const char c)
{
    return iax_send_dtmf(session, c);
}


Codec *IAX::get_codec(int format)
{
    if (codec_map.count(format) < 1)
    {
        switch (format)
        {
            case AST_FORMAT_GSM:
                codec_map[format] = new GSM();
                break;
            default:
                fprintf(stderr,"Unknown format %d in IAX::get_codec()\n");
                return 0;
        }
    }
    return codec_map[format];
}
