#include "iax.h"

IAXClient::IAXClient()
{
    port = iax_init(4569);
    session = iax_session_new();
}
IAXClient::~IAXClient()
{
    iax_destroy(session);
}
