#ifndef AUDIO_H
#define AUDIO_H

#include "ringbuffer.h"

class AudioInterface
{
    public:
        AudioInterface();
        ~AudioInterface();

        /* Fill these ringbuffers with 8000Hz 16-bit data. */
        jack_ringbuffer_t *input_rb;	// i.e. mouthpiece
        jack_ringbuffer_t *output_rb;	// i.e. earpiece

        int off_hook;
};
#endif // ifndef AUDIO_H
