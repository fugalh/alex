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


