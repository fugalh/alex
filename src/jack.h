#ifndef ALEX_JACK_H
#define ALEX_JACK_H

#include "audio.h"
#include <jack/jack.h>

class Jack : public AudioInterface
{
    public:
        Jack(const char *client_name);
        ~Jack();

    private:
        static int jack_process_wrapper(jack_nframes_t nframes, void *arg);
        static void jack_shutdown_wrapper(void *arg);

        int jack_process(jack_nframes_t nframes, void *arg);
        void jack_shutdown(void *arg);
        jack_port_t *input_port;
        jack_port_t *output_port;
        jack_client_t *client;
};

#endif
