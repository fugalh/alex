#include <jack/ringbuffer.h>

#define SAMPLERATE 8000

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
