#include "iax.h"

IAXClient::IAXClient(AudioInterface *a, Coder *c) : 
    audio(a), coder(c)
{
    port = iax_init(IAX_DEFAULT_PORTNO);
    session = iax_session_new();
}

IAXClient::~IAXClient()
{
    iax_destroy(session);
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

