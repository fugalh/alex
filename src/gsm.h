#ifndef ALEX_GSM_H
#define ALEX_GSM_H

#include "coder.h"
#include <gsm.h>

class GSMCoder : public Coder
{
    public:
        GSMCoder() { handle = gsm_create(); }
        ~GSMCoder() { gsm_destroy(handle); }
        int encode(jack_ringbuffer_t *in);
        int decode(jack_ringbuffer_t *in);

    protected:
        gsm handle;
	static int format;
};

#endif
