#ifndef ALEX_IAX_H
#define ALEX_IAX_H

#include "audio.h"
#include "coder.h"

#include <stddef.h> // for size_t in <iax/iax-client.h>
#include <iax/iax-client.h>

class IAXClient
{
    public:
        IAXClient(AudioInterface *, Coder*);
        ~IAXClient();


    protected:
        struct iax_session *session;
        int port;
        AudioInterface *audio;
        Coder *coder;
};

#endif
