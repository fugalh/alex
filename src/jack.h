#include "audio.h"

class Jack : public AudioInterface
{
    public:
        static Jack* instance();
        ~Jack();

    protected:
        Jack();

    private:
        static int jack_process(jack_nframes_t nframes, void *arg);
        static void jack_shutdown(void *arg);
        static Jack* _instance;
        jack_port_t *input_port;
        jack_port_t *output_port;
        jack_client_t *client;
};
