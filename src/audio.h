#ifndef AUDIO_H
#define AUDIO_H

#include "ringbuffer.h"
#include <samplerate.h>

class AudioInterface
{
    public:
        AudioInterface();
        ~AudioInterface();

        /* Fill these ringbuffers with 8000Hz 16-bit data. */
        jack_ringbuffer_t *input_rb;	// i.e. mouthpiece
        jack_ringbuffer_t *output_rb;	// i.e. earpiece

        int off_hook;
	SRC_STATE *src_input_state;
	SRC_STATE *src_output_state;
};
#endif // ifndef AUDIO_H
