#include "iax.h"
#include "threads.h"

sem_t event_sem;

IAXClient::IAXClient(AudioInterface *a, Coder *c) : 
    audio(a), coder(c)
{
    sem_init(&event_sem,0,0);
    port = iax_init(IAX_DEFAULT_PORTNO);
    session = iax_session_new();
    gsm_handle = gsm_create();
    iax_event_rb = jack_ringbuffer_create(sizeof(struct iax_event*) * 10000);
}

IAXClient::~IAXClient()
{
    jack_ringbuffer_free(iax_event_rb);
    iax_destroy(session);
    sem_destroy(&event_sem);
    gsm_destroy(gsm_handle);
}


int IAXClient::call(char* cidnum, char* cidname, char* ich)
{
    int ret = iax_call(session, cidnum, cidname, ich, 
	    NULL, 1, AST_FORMAT_GSM, AST_FORMAT_GSM);
    audio->off_hook = 1;
    return ret;
}

int IAXClient::hangup(char* byemsg)
{
    int ret = iax_hangup(session, byemsg);
    audio->off_hook = 0;
    return ret;
}

void *iax_network_loop(void *arg)
{
    IAXClient *iax = (IAXClient*)arg;
    jack_ringbuffer_t *rb = iax->iax_event_rb;
    while(1)
    {
	struct iax_event *ev = iax_get_event(1);
	if (!ev) continue; 

	if (jack_ringbuffer_write_space(rb) >= sizeof(ev))
	{
	    jack_ringbuffer_write(rb, (char*)&ev, sizeof(ev));
	    sem_post(&event_sem);
	}
	else
	{
	    // XXX somebody do something!
	    fprintf(stderr, "rb full, dropping iax_event and quitting!\n");
	    return 0;
	}
    }
    return 0;
}

void IAXClient::event_loop()
{
    while (1)
    {
        sem_wait(&event_sem);
        if (jack_ringbuffer_read_space(iax_event_rb) > 0)
        {
            struct iax_event *ev;
            jack_ringbuffer_read(iax_event_rb, (char*)&ev, sizeof(ev));

	    switch (ev->etype)
	    {
		case IAX_EVENT_ACCEPT:
		    printf("Call accepted.\n");
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
        while (jack_ringbuffer_read_space(audio->input_rb) >= 33)
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
    }
}

void *iax_event_loop(void *arg)
{
    IAXClient *iax = (IAXClient*)arg;

    pthread_t network_loop_thread;
    pthread_create(&network_loop_thread, 0, iax_network_loop, iax);

    iax->event_loop();
    return 0;
}

int IAXClient::handle_voice(struct iax_event *ev)
{
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
		    fprintf(stderr,"Buffer overflow in handle_voice(): "
			    "%d bytes.\n", sizeof(dst) - ret);
	    }
	    break;
	}
	default:
	    fprintf(stderr,"Unknown voice format %d.\n", ev->subclass);
	    return 1;
    }
    return 0;
}

int IAXClient::dtmf(const char c)
{
    return iax_send_dtmf(session, c);
}
