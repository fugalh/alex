#include "iax.h"

IAXClient::IAXClient(AudioInterface *a, Coder *c) : audio(a), coder(c)
{
    port = iax_init(4569);
    session = iax_session_new();
}
IAXClient::~IAXClient()
{
    iax_destroy(session);
}
