#include "audio.h"

AudioInterface::AudioInterface() : off_hook(0)
{
    /* Set up the ringbuffers. 
     * Ensure that they are evenly divisible by 320 (160 * sizeof(gsm_signal))
     * so that we will never have to worry about gsm_encode wrapping around
     * the ring buffer. */
    input_rb  = jack_ringbuffer_create(SAMPLERATE * 5); // 5 seconds
    output_rb = jack_ringbuffer_create(SAMPLERATE * 5); // 5 seconds
}

AudioInterface::~AudioInterface()
{
    /* free ringbuffers */
    jack_ringbuffer_free(input_rb);
    jack_ringbuffer_free(output_rb);
}
