#include "iax.h"
#include "threads.h"

sem_t event_sem;

IAXClient::IAXClient(AudioInterface *a, Coder *c) : 
    audio(a), coder(c)
{
    sem_init(&event_sem,0,0);
    port = iax_init(IAX_DEFAULT_PORTNO);
    session = iax_session_new();
    iax_event_rb = jack_ringbuffer_create(sizeof(struct iax_event) * 1000);
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
    return ret;
}

int IAXClient::hangup(char* byemsg)
{
    int ret = iax_hangup(session, byemsg);
    return ret;
}

void *iax_event_thread_func(void *arg)
{
    IAXClient *iax = (IAXClient*)arg;
    jack_ringbuffer_t *rb = iax->iax_event_rb;
    while(1)
    {
	struct iax_event *ev = iax_get_event(1);

	/*
	if (jack_ringbuffer_write_space(rb) < sizeof(*ev))
	{
	    jack_ringbuffer_write(rb, (char*)ev, sizeof(*ev));
	    sem_post(&event_sem);
	}
	else
	{
	    // XXX somebody do something!
	    fprintf(stderr, "rb full, dropping iax_event and quitting!\n");
	    return 0;
	}
	*/

	iax_event_free(ev);
    }
    return 0;
}
