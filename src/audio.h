#ifndef AUDIO_H
#define AUDIO_H

#include "ringbuffer.h"
#include <samplerate.h>

class Audio
{
    public:
        Audio();
        virtual ~Audio();

	jack_ringbuffer_t *irb; /// input (mic) ringbuffer
	jack_ringbuffer_t *orb; /// output (speaker) ringbuffer
	/// Semaphore read file descriptor, to select on.
	int semrfd;

        int off_hook;

    protected:
};
#endif // ifndef AUDIO_H
