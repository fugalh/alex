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
#define min(X, Y)  ((X) < (Y) ? (X) : (Y))

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
    jack_start();
}

Jack::~Jack()
{
    src_delete(src_state_input);
    src_delete(src_state_output);
    jack_stop();
}

int Jack::read(short *buf, int count)
{
    SRC_DATA *d = &src_data_input;

    // XXX Think about doing this with vec
    int fbuf_len = min((unsigned int)count,jack_ringbuffer_read_space(jirb)/sizeof(sample_t));
    int fbuf_size = fbuf_len * sizeof(sample_t);
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
    sample_t *dst = (sample_t*)alloca(count*sizeof(sample_t));
    SRC_DATA *d = &src_data_output;
    d->data_in = fbuf;
    d->input_frames = count;
    d->data_out = dst;
    d->output_frames = count;
    d->src_ratio = samplerate / 8000.0;
    d->end_of_input = 0;
    int ret = src_process(src_state_output, d);
    if (ret != 0)
    {
        fprintf(stderr,"write(): %s\n",src_strerror(ret));
        return 0;
    }

    // to ringbuffer
    int len = min((unsigned int)d->input_frames_used, jack_ringbuffer_write_space(jorb)/sizeof(sample_t));
    jack_ringbuffer_write(jorb, (char*)dst, len*sizeof(sample_t));

    return len;
}

#if 0
int Jack::jack_process(jack_nframes_t nframes, void *arg)
{
    int ret;
    jack_ringbuffer_data_t vec[2];
    float *bp, *ep;
    SRC_DATA data;
    sample_t *in  = (sample_t*) jack_port_get_buffer (input_port, nframes);
    sample_t *out = (sample_t*) jack_port_get_buffer (output_port, nframes);
    memset(out,0,sizeof(sample_t)*nframes);

    // output
    data.src_ratio = jack_get_sample_rate(client) / 8000.0;

    bp = buf;
    ep = buf + min(
	    (int)ceil(nframes/data.src_ratio), // resamples to nframes
	    jack_ringbuffer_read_space(output_rb)/sizeof(short));
    jack_ringbuffer_get_read_vector(output_rb, vec);
    for (int i=0; i<2 && bp<ep; i++)
    {
	// output_rb->buf (short to float)
	int len = min(ep-bp,vec[i].len);
	src_short_to_float_array((short*)vec[i].buf, bp, len);
	bp += len;
    }
    int len = bp - buf;
    jack_ringbuffer_read_advance(output_rb, len*sizeof(short));

    // buf->out (resample)
    data.data_in = buf;
    data.input_frames = len;
    data.data_out = out;
    data.output_frames = nframes;
    data.end_of_input = 0;
    ret = src_process(src_output_state, &data);
    if (ret != 0)
    {
	fprintf(stderr,"%s\n",src_strerror(ret));
	return 1;
    }

    // input (in->buf->input_rb)
    if (off_hook)
    {
	// in->buf (resample)
	int rb_frames = jack_ringbuffer_write_space(input_rb)/sizeof(short);
	data.data_in = in;
	data.input_frames = nframes;
	data.data_out = buf;
	data.output_frames = rb_frames;
	data.src_ratio = 8000.0 / jack_get_sample_rate(client);
	data.end_of_input = 0;
	ret = src_process(src_input_state, &data);
	if (ret != 0)
	{
	    fprintf(stderr,"%s\n",src_strerror(ret));
	    return 1;
	}
	//printf("%d %d ",data.input_frames_used, data.output_frames_gen);

	//normalize(buf,data.output_frames_gen);

	// buf->input_rb (float to short)
	float *bp, *ep;
	bp = buf;
	ep = buf + min(rb_frames, data.output_frames_gen);
	jack_ringbuffer_get_write_vector(input_rb, vec);
	for (int i=0; i<2 && bp<ep; i++)
	{
	    int len = min(vec[i].len/sizeof(short), ep-bp);
	    src_float_to_short_array(bp, (short*)(vec[i].buf), len);
	    bp += len;
	}
	//printf("%d \n",bp-buf);
	jack_ringbuffer_write_advance(input_rb, (bp-buf)*sizeof(short));
        sem_post(&event_sem);
    }


    return 0;      
}
#endif
