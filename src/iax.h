#ifndef ALEX_IAX_H
#define ALEX_IAX_H

#include "protocol.h"
#include "audio.h"
#include "codec.h"

#include <stddef.h> // for size_t in <iax/iax-client.h>
#include <iax/iax-client.h>
#include <pthread.h>
#include <gsm.h>
#include <map>
using std::map;

class IAX : public Protocol
{
    public:
        IAX(Audio*);
        virtual ~IAX();

	// NB: ich = IAX Call Handle (e.g. user:secret@host/extension)
	int call(char* cidnum, char* cidname, char* ich);
	int answer();
	int hangup(char* byemsg);
	int dtmf(const char);
	int reg(char* hostname, char* peer, char* secret, int refresh);
	int quelch();
	int unquelch();

        Codec *get_codec(int format /* e.g. AST_FORMAT_GSM */);

        /// Called by the thread function
        void event_loop();

    protected:
        pthread_t thread;
        pthread_mutex_t session_mutex;
        struct iax_session *session;
        int port;

	int handle_voice(struct iax_event*);
        map<int,Codec*> codec_map;
};

#endif
