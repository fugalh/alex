#ifndef ALEX_JACK_H
#define ALEX_JACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jack/jack.h>
#include <jack/ringbuffer.h>

    void jack_start();
    void jack_stop();

    extern jack_ringbuffer_t *jirb;
    extern jack_ringbuffer_t *jorb;
    extern int *jack_off_hook;
    extern int samplerate;
    extern int semwfd;
    extern int semrfd;

    typedef jack_default_audio_sample_t sample_t;

#ifdef __cplusplus
}

#include "audio.h"
#include <samplerate.h>

class Jack : public Audio
{
    public:
        Jack();
        ~Jack();

        int read(short *buf, int count);
        int write(short *buf, int count);

    private:
        SRC_STATE *src_state_input;
        SRC_STATE *src_state_output;
        SRC_DATA src_data_input;
        SRC_DATA src_data_output;
};

#endif // __cplusplus

#endif
