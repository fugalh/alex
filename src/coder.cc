#include "coder.h"
Coder::Coder()
{
    encoded_rb = jack_ringbuffer_create(RB_SIZE);
    decoded_rb = jack_ringbuffer_create(RB_SIZE);
}
Coder::~Coder()
{
    jack_ringbuffer_free(encoded_rb);
    jack_ringbuffer_free(decoded_rb);
}
