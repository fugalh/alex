#include "coder.h"
#include <gsm.h>

class GSMCoder : public Coder
{
    public:
        GSMCoder() { handle = gsm_create(); }
        ~GSMCoder() { gsm_destroy(handle); }
        virtual int encode(jack_ringbuffer_t *in, jack_ringbuffer_t *out);
        virtual int decode(jack_ringbuffer_t *in, jack_ringbuffer_t *out);

    protected:
        gsm handle;
};
