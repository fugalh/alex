#ifndef ALEX_JACK_H
#define ALEX_JACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <samplerate.h>

    void jack_start(void *arg);
    void jack_stop();

    extern jack_ringbuffer_t *jirb;
    extern jack_ringbuffer_t *jorb;
    extern int *jack_off_hook;
    extern int samplerate;
    extern int semwfd;

    // libsamplerate stuff
    extern SRC_STATE *src_state_input;
    extern SRC_STATE *src_state_output;
    extern SRC_DATA src_data_input;
    extern SRC_DATA src_data_output;

    typedef jack_default_audio_sample_t sample_t;

#ifdef __cplusplus
}

#include "audio.h"

class Jack : public Audio
{
    public:
        Jack();
        ~Jack();
};

#endif // __cplusplus

#endif
