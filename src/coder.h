#include <jack/ringbuffer.h>

class Coder
{
    public:
        virtual int encode(jack_ringbuffer_t *in, jack_ringbuffer_t *out) = 0;
        virtual int decode(jack_ringbuffer_t *in, jack_ringbuffer_t *out) = 0;
};
