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

Jack::Jack(const char *client_name)
{
    const char **ports;

    buf = (sample_t*)malloc(48000 * sizeof(sample_t));

    /* try to become a client of the JACK server */

    if ((client = jack_client_new (client_name)) == 0) 
    {
	fprintf (stderr, "jack server not running?\n");
	exit(1); // XXX maybe an exception?
    }

    /* tell the JACK server to call `process()' whenever
       there is work to be done.
       */

    jack_set_process_callback (client, &jack_process_wrapper, this);

    /* tell the JACK server to call `jack_shutdown()' if
       it ever shuts down, either entirely, or if it
       just decides to stop calling us.
       */

    //jack_on_shutdown (client, &jack_shutdown_wrapper, this);

    /* display the current sample rate. 
    */

    printf ("engine sample rate: %lu\n",
	    jack_get_sample_rate (client));

    /* create two ports */

    input_port = jack_port_register (client, "input", 
            JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    output_port = jack_port_register (client, "output", 
            JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    /* tell the JACK server that we are ready to roll */

    if (jack_activate (client)) 
    {
	fprintf (stderr, "cannot activate client");
	exit(1); // XXX Exception
    }

    /* connect the ports. Note: you can't do this before
       the client is activated, because we can't allow
       connections to be made to clients that aren't
       running.
       */

    if ((ports = jack_get_ports (client, NULL, NULL, 
                    JackPortIsPhysical|JackPortIsOutput)) == NULL) 
    {
	fprintf(stderr, "Cannot find any physical capture ports\n");
	//exit(1);
    }
    else 
    {
	if (jack_connect (client, ports[0], jack_port_name (input_port))) 
	{
	    fprintf (stderr, "cannot connect input ports\n");
	}
	free (ports);
    }


    if ((ports = jack_get_ports (client, NULL, NULL, 
                    JackPortIsPhysical|JackPortIsInput)) == NULL) 
    {
	fprintf(stderr, "Cannot find any physical playback ports\n");
	//exit(1);
    } 
    else 
    {
	if (jack_connect (client, jack_port_name (output_port), ports[0])) 
	{
	    fprintf (stderr, "cannot connect output ports\n");
	}
	free (ports);
    }
}

Jack::~Jack()
{
    jack_deactivate(client);
    jack_client_close (client);
    free(buf);
}

/**
 * The process callback for this JACK application.
 * It is called by JACK at the appropriate times.
 */
int Jack::jack_process_wrapper(jack_nframes_t nframes, void *arg)
{
    return ((Jack*)arg)->jack_process(nframes,arg);
}

void normalize(float *buf, int nframes)
{
    float max,min;
    max = min = 0;
    for (int i=0; i<nframes; i++)
    {
	float s = *(buf+i);
	if (max < s)
	    max = s;
	if (min > s)
	    min = s;
    }
    if (max > 1.0 || min < -1.0)
	printf("*** %f %f\n",max,min);
}

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

void Jack::jack_shutdown_wrapper(void *arg)
{
    ((Jack*)arg)->jack_shutdown(arg);
}

void Jack::jack_shutdown(void *arg)
{
    // raise an exception or something
    exit(1);
}
