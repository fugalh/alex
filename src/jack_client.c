/* Based on simple_client.c from the jack distribution.  */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "jack.h"
#include "util.h"

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;
jack_ringbuffer_t *jirb; // Jack input ringbuffer
jack_ringbuffer_t *jorb; // Jack output ringbuffer
int *jack_off_hook;
int samplerate;
SRC_STATE *src_state_input;
SRC_STATE *src_state_output;
SRC_DATA src_data_input;
SRC_DATA src_data_output;

/**
 * The process callback for this JACK application.
 * It is called by JACK at the appropriate times.
 */
int
process (jack_nframes_t nframes, void *arg)
{
    sample_t *out = (sample_t *) jack_port_get_buffer (output_port, nframes);
    sample_t *in = (sample_t *) jack_port_get_buffer (input_port, nframes);
    samplerate = jack_get_sample_rate(client);

    // input
    if (*jack_off_hook)
    {
	SRC_DATA *d = &src_data_input;

	// resample
	d->src_ratio = 8000.0 / samplerate;
	int dstlen = (int)ceil(d->src_ratio) * nframes;
	sample_t *fbuf2 = (sample_t*)alloca(dstlen*sizeof(sample_t));
	d->data_in = in;
	d->input_frames = nframes;
	d->data_out = fbuf2;
	d->output_frames = dstlen;
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
	src_float_to_short_array(fbuf2, sbuf, len);

	// ringbuffer
	jack_ringbuffer_write(jirb, (char*)sbuf, len*sizeof(short));

	// signal 
	int foo = 1;
	write(semwfd, &foo, 1);
    }

    // output
    {
	memset(out,0,nframes);

	// ringbuffer
	int count = min(jack_ringbuffer_read_space(jorb)/sizeof(short),nframes);
	short *sbuf = (short*)alloca(count*sizeof(short));
	count = jack_ringbuffer_read(jorb, (char*)sbuf, count*sizeof(short));
	count /= sizeof(short);
	if (count < 1) return 0;

	// short to float
	sample_t *fbuf = (sample_t*)alloca(count*sizeof(sample_t));
	src_short_to_float_array(sbuf, fbuf, count);

	// resample
	SRC_DATA *d = &src_data_output;
	d->data_in = fbuf;
	d->input_frames = count;
	d->data_out = out;
	d->output_frames = nframes;
	d->src_ratio = samplerate / 8000.0;
	d->end_of_input = 0;
	int ret = src_process(src_state_output, d);
	if (ret != 0)
	{
	    fprintf(stderr,"write(): %s\n",src_strerror(ret));
	    return 0;
	}
    }

    return 0;      
}

/**
 * This is the shutdown callback for this JACK application.
 * It is called by JACK if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown (void *arg)
{
    fprintf(stderr,"jack_shutdown()\n");
    //exit (1);
}

void jack_start(void *arg)
{
    const char **ports;
    int err;

    src_state_input = src_new(SRC_SINC_BEST_QUALITY, 1, &err);
    if (!src_state_input)
    {
	fprintf(stderr,"Error creating src_state_input: %s\n",src_strerror(err));
    }
    src_state_output = src_new(SRC_SINC_BEST_QUALITY, 1, &err);
    if (!src_state_output)
    {
	fprintf(stderr,"Error creating src_state_output: %s\n",src_strerror(err));
    }

    /* try to become a client of the JACK server */

    if ((client = jack_client_new ("alex")) == 0) {
	fprintf (stderr, "jack server not running?\n");
	exit(1);
    }

    /* tell the JACK server to call `process()' whenever
       there is work to be done.
       */

    jack_set_process_callback (client, process, arg);

    /* tell the JACK server to call `jack_shutdown()' if
       it ever shuts down, either entirely, or if it
       just decides to stop calling us.
       */

    //jack_on_shutdown (client, jack_shutdown, arg);

    /* display the current sample rate. 
    */

    /*
       printf ("engine sample rate: %" PRIu32 "\n",
       jack_get_sample_rate (client));
       */

    /* create two ports */

    input_port = jack_port_register (client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    output_port = jack_port_register (client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    /* tell the JACK server that we are ready to roll */

    if (jack_activate (client)) {
	fprintf (stderr, "cannot activate client");
	return;
    }

    /* connect the ports. Note: you can't do this before
       the client is activated, because we can't allow
       connections to be made to clients that aren't
       running.
       */

    if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == NULL) {
	fprintf(stderr, "Cannot find any physical capture ports\n");
	//exit(1);
    }

    if (jack_connect (client, ports[0], jack_port_name (input_port))) {
	fprintf (stderr, "cannot connect input ports\n");
    }

    free (ports);

    if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
	fprintf(stderr, "Cannot find any physical playback ports\n");
	//exit(1);
    }

    if (jack_connect (client, jack_port_name (output_port), ports[0])) {
	fprintf (stderr, "cannot connect output ports\n");
    }

    free (ports);

    /* Since this is just a toy, run for a few seconds, then finish */
}

void jack_stop()
{
    jack_deactivate(client);
    jack_client_close (client);
    src_delete(src_state_input);
    src_delete(src_state_output);
    jack_ringbuffer_free(jorb);
    jack_ringbuffer_free(jirb);
}
