#ifndef AUDIO_H
#define AUDIO_H

#include "ringbuffer.h"
#include <samplerate.h>

class Audio
{
    public:
        Audio();
        virtual ~Audio();

        virtual int read(short *buf, int count) = 0;
        virtual int write(short *buf, int count) = 0;
        int off_hook;

	/// Semaphore read file descriptor, to select on.
	int semrfd;

    protected:
	SRC_STATE *src_input_state;
	SRC_STATE *src_output_state;
};
#endif // ifndef AUDIO_H
