#include "audio.h"
#include <stdio.h>
#include <stdlib.h>

AudioInterface::AudioInterface() : off_hook(0)
{
    /* Set up the ringbuffers. 
     * Ensure that they are evenly divisible by 320 (160 * sizeof(gsm_signal))
     * so that we will never have to worry about gsm_encode wrapping around
     * the ring buffer. */
    input_rb  = jack_ringbuffer_create(RB_SIZE);
    output_rb = jack_ringbuffer_create(RB_SIZE);
    int err;
    src_input_state = src_new(SRC_SINC_BEST_QUALITY, 1, &err);
    if (src_input_state == 0)
    {
	fprintf(stderr,"%s\n",src_strerror(err));
	exit(1);
    }
    src_output_state = src_new(SRC_SINC_BEST_QUALITY, 1, &err);
    if (src_output_state == 0)
    {
	fprintf(stderr,"%s\n",src_strerror(err));
	exit(1);
    }
}

AudioInterface::~AudioInterface()
{
    /* free ringbuffers */
    jack_ringbuffer_free(input_rb);
    jack_ringbuffer_free(output_rb);
    src_delete(src_input_state);
    src_delete(src_output_state);
}
