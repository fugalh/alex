#include "audio.h"
#include <stdio.h>
#include <stdlib.h>

Audio::Audio() : off_hook(0)
{
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

Audio::~Audio()
{
    /* free ringbuffers */
    src_delete(src_input_state);
    src_delete(src_output_state);
}
