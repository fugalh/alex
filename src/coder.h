#ifndef CODER_H
#define CODER_H

#include "ringbuffer.h"

class Coder
{
    public:
        Coder();
        ~Coder();
        // output in encoded_rb
        virtual int encode(jack_ringbuffer_t *in) = 0;
        // output in decoded_rb
        virtual int decode(jack_ringbuffer_t *in) = 0;

        jack_ringbuffer_t *encoded_rb;
        jack_ringbuffer_t *decoded_rb;
};

#endif
