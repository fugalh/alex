//#include <iax/iax-client.h>
#include <stddef.h>
#include <iax/iax-client.h>

class IAXClient
{
    public:
        IAXClient();
        ~IAXClient();


    protected:
        struct iax_session *session;
        int port;
};
