#ifndef ALEX_IAX_H
#define ALEX_IAX_H

#include "protocol.h"
#include "audio.h"
#include "codec.h"

#include <stddef.h> // for size_t in <iax/iax-client.h>
#include <iax/iax-client.h>
#include <string>
#include <pthread.h>
#include <gsm.h>

using std::string;

class IAX2Protocol : public Protocol
{
    public:
        IAX2Protocol(Audio*, int format);
        ~IAX2Protocol();

	// NB: ich = IAX Call Handle (e.g. user:secret@host/extension)
	int call(char* cidnum, char* cidname, char* ich);
	int answer();
	int hangup(char* byemsg);
	int dtmf(const char);
	int reg(char* hostname, char* peer, char* secret, int refresh);
	int quelch();
	int unquelch();

        void event_loop();
        void set_codec(int /* e.g. AST_FORMAT_GSM */);

        Audio *audio;
    protected:
        pthread_t thread;
        pthread_mutex_t session_mutex;
        struct iax_session *session;
        int port;

	int handle_voice(struct iax_event*);
        gsm gsm_handle;
        int codec_format; // e.g. AST_FORMAT_GSM
        Codec *codec;
};

#endif
