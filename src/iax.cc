#include "iax.h"
#include "threads.h"

pthread_mutex_t iax_event_queue_mut = PTHREAD_MUTEX_INITIALIZER;
sem_t event_sem;

IAXClient::IAXClient(AudioInterface *a, Coder *c) : 
    audio(a), coder(c)
{
    port = iax_init(IAX_DEFAULT_PORTNO);
    session = iax_session_new();
    sem_init(&event_sem,0,0)
}

IAXClient::~IAXClient()
{
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

