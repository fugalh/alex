#include "iax.h"
#include "threads.h"

sem_t event_sem;

IAXClient::IAXClient(AudioInterface *a, Coder *c) : 
    audio(a), coder(c)
{
    sem_init(&event_sem,0,0);
    port = iax_init(IAX_DEFAULT_PORTNO);
    session = iax_session_new();
    iax_event_rb = jack_ringbuffer_create(sizeof(struct iax_event*) * 10000);
}

IAXClient::~IAXClient()
{
    jack_ringbuffer_free(iax_event_rb);
    iax_destroy(session);
    sem_destroy(&event_sem);
}


int IAXClient::call(char* cidnum, char* cidname, char* ich)
{
    int ret = iax_call(session, cidnum, cidname, ich, 
	    NULL, 0, coder->format, 0);
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
	    printf("2. ev: %d\n",ev);

	    /*
	    if (ev->etype == IAX_EVENT_ACCEPT)
	    {
		iax_accept(session, coder->format);
	    }
	    */

            iax_event_free(ev);
        }
	int n;
        if ((n = jack_ringbuffer_read_space(audio->input_rb)) > 0)
        {
	    jack_ringbuffer_read_advance(audio->input_rb, n); // temporary
            //encode
            //send
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
