/* Based on simple_client.c from the JACK distribution */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "jack.h"
#include "threads.h"
#include <samplerate.h>
#include "util.h"

int semwfd;
int semrfd;

Jack::Jack()
{
    jack_off_hook = &off_hook;
    int err;
    src_state_input = src_new(SRC_SINC_BEST_QUALITY, 1, &err);
    if (!src_state_input)
    {
        fprintf(stderr,"Error creating src_state_input: %s\n",src_strerror(err));
        // raise an exception?
    }
    src_state_output = src_new(SRC_SINC_BEST_QUALITY, 1, &err);
    if (!src_state_output)
    {
        fprintf(stderr,"Error creating src_state_output: %s\n",src_strerror(err));
        // raise an exception?
    }
    jirb = jack_ringbuffer_create(96000);
    jorb = jack_ringbuffer_create(96000);
    int filedes[2];
    pipe(filedes);
    semrfd = filedes[0];
    semwfd = filedes[1];
    jack_start();
}

Jack::~Jack()
{
    src_delete(src_state_input);
    src_delete(src_state_output);
    jack_stop();
    jack_ringbuffer_free(jorb);
    jack_ringbuffer_free(jirb);
}

int Jack::read(short *buf, int count)
{
    SRC_DATA *d = &src_data_input;

    // XXX Think about doing this with vec
    int fbuf_len = min((unsigned int)count,jack_ringbuffer_read_space(jirb)/sizeof(sample_t));
    int fbuf_size = fbuf_len * sizeof(sample_t);
    if (fbuf_size <= 0)
	return 0;
    sample_t *fbuf = (sample_t*)alloca(fbuf_size);
    jack_ringbuffer_read(jirb, (char*)fbuf, fbuf_size);

    // resample
    sample_t *fbuf2 = (sample_t*)alloca(count*sizeof(sample_t));
    d->data_in = fbuf;
    d->input_frames = fbuf_len;
    d->data_out = fbuf2;
    d->output_frames = count;
    d->src_ratio = 8000.0 / samplerate;
    d->end_of_input = 0;
    int ret = src_process(src_state_input, d);
    if (ret != 0)
    {
        fprintf(stderr,"read(): %s\n",src_strerror(ret));
        return 0;
    }

    // float to short
    int len = d->output_frames_gen;
    short *sbuf = (short*)alloca(len*sizeof(short));
    src_float_to_short_array(fbuf, sbuf, len);

    return len;
}

int Jack::write(short *buf, int count)
{
    // short to float
    sample_t *fbuf = (sample_t*)alloca(count*sizeof(sample_t));
    src_short_to_float_array(buf, fbuf, count);

    // resample
    double ratio = samplerate / 8000.0;
    int dstlen = (int)ceil(ratio) * count;
    sample_t *dst = (sample_t*)alloca(dstlen*sizeof(sample_t));
    SRC_DATA *d = &src_data_output;
    d->data_in = fbuf;
    d->input_frames = count;
    d->data_out = dst;
    d->output_frames = dstlen;
    d->src_ratio = ratio;
    d->end_of_input = 0;
    int ret = src_process(src_state_output, d);
    if (ret != 0)
    {
        fprintf(stderr,"write(): %s\n",src_strerror(ret));
        return 0;
    }

    // to ringbuffer
    int len = min((unsigned int)d->output_frames_gen, jack_ringbuffer_write_space(jorb)/sizeof(sample_t));
    jack_ringbuffer_write(jorb, (char*)dst, len*sizeof(sample_t));

    return len;
}
