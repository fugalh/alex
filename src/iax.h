#ifndef ALEX_IAX_H
#define ALEX_IAX_H

#include "audio.h"
#include "coder.h"

#include <stddef.h> // for size_t in <iax/iax-client.h>
#include <iax/iax-client.h>
#include <string>
#include <pthread.h>

using std::string;

class IAXClient
{
    public:
        IAXClient(AudioInterface*, Coder*);
        ~IAXClient();

	// NB: ich = IAX Call Handle (e.g. user:secret@host/extension)
	int call(char* cidnum, char* cidname, char* ich);
	int answer();
	int hangup(char* byemsg);
	int dtmf(const char);
	int reg(char* hostname, char* peer, char* secret, int refresh);
	int quelch();
	int unquelch();

        void event_loop();
	jack_ringbuffer_t *iax_event_rb;

    protected:
        pthread_mutex_t session_mutex;
        struct iax_session *session;
        int port;
        AudioInterface *audio;
        Coder *coder;
};
void *iax_event_thread_func(void *arg);

#endif
