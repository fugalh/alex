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

Jack::Jack(const char *client_name)
{
    const char **ports;

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
}

/**
 * The process callback for this JACK application.
 * It is called by JACK at the appropriate times.
 */
int Jack::jack_process_wrapper(jack_nframes_t nframes, void *arg)
{
    return ((Jack*)arg)->jack_process(nframes,arg);
}

typedef jack_default_audio_sample_t sample_t;
int Jack::jack_process(jack_nframes_t nframes, void *arg)
{
    int ret;
    jack_ringbuffer_data_t vec[2];
    int n = sizeof(sample_t) * nframes;
    sample_t *in  = (sample_t*) jack_port_get_buffer (input_port, nframes);
    sample_t *out = (sample_t*) jack_port_get_buffer (output_port, nframes);

    if (!off_hook)
    {
        // enjoy the silence
        memset(out,0,n);
        return 0;
    }

    // input
    SRC_DATA data;
    data.src_ratio = 8000.0 / jack_get_sample_rate(client);
    sample_t *buf = (sample_t*)alloca(n); // XXX want to use alloca?
    int framecount = 0;
    jack_ringbuffer_get_write_vector(input_rb, vec);
    for (int i=0; i<2 && vec[i].len>0; i++)
    {

	data.data_in = in + framecount;
	data.input_frames = nframes - framecount;
	data.data_out = buf;
	data.output_frames = vec[i].len / sizeof(short);

	src_simple(&data, SRC_SINC_BEST_QUALITY, 1);
	src_float_to_short_array((float*)(data.data_out), (short*)(vec[i].buf),
		data.output_frames_gen);

	jack_ringbuffer_write_advance(input_rb, 
		data.output_frames_gen * sizeof(short));
	framecount += data.input_frames_used;
    }
    if (framecount < n) { /* somebody do something! */ }
    sem_post(&event_sem);

    // output
    memset(out,0,n);
    data.src_ratio = (double)jack_get_sample_rate(client) / 8000;
    framecount = 0;
    jack_ringbuffer_get_write_vector(output_rb, vec);
    for (int i=0; i<2 && vec[i].len>0; i++)
    {
	data.data_in = buf;
	data.input_frames = vec[i].len / sizeof(short);
	data.data_out = out + framecount;
	data.output_frames = nframes - framecount;

	src_short_to_float_array((short*)(vec[i].buf), (float*)(data.data_in),
		vec[i].len / sizeof(short));
	src_simple(&data, SRC_SINC_BEST_QUALITY, 1);

	jack_ringbuffer_read_advance(output_rb, 
		data.input_frames_used * sizeof(short));
	framecount += data.output_frames_gen;
    }
    ret = jack_ringbuffer_read(output_rb, (char*)out, n);
    if (ret < n) { /* somebody do something! */ }

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
