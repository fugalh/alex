/** @file simple_client.c
 *
 * @brief This simple client demonstrates the basic features of JACK
 * as they would be used by many applications.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>
#include "audio.h"

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;

/**
 * The process callback for this JACK application.
 * It is called by JACK at the appropriate times.
 */
int
process (jack_nframes_t nframes, void *arg)
{  
    int n = sizeof (jack_default_audio_sample_t) * nframes;
    int ret;
    // input
    jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) 
	jack_port_get_buffer (input_port, nframes);
    ret = jack_ringbuffer_write(input_rb, in, n);
    if (ret < n) { /* somebody do something! */ }

    // output
    jack_default_audio_sample_t *out = (jack_default_audio_sample_t *) 
	jack_port_get_buffer (output_port, nframes);
    ret = jack_ringbuffer_read(output_rb, out, n);
    if (ret < n) { /* somebody do something! */ }

    return 0;      
}

/**
 * This is the shutdown callback for this JACK application.
 * It is called by JACK if the server ever shuts down or
 * decides to disconnect the client.
 */
void
jack_shutdown (void *arg)
{
    fprintf(stderr,"JACK shutdown");
    exit (1);
}

int
main (int argc, char *argv[])
{
    const char **ports;

    /* try to become a client of the JACK server */

    if ((client = jack_client_new ("cliax")) == 0) {
	fprintf (stderr, "jack server not running?\n");
	return 1;
    }

    /* tell the JACK server to call `process()' whenever
       there is work to be done.
       */

    jack_set_process_callback (client, process, 0);

    /* tell the JACK server to call `jack_shutdown()' if
       it ever shuts down, either entirely, or if it
       just decides to stop calling us.
       */

    jack_on_shutdown (client, jack_shutdown, 0);

    /* display the current sample rate. 
    */

    printf ("engine sample rate: %" PRIu32 "\n",
	    jack_get_sample_rate (client));

    /* create two ports */

    input_port = jack_port_register (client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    output_port = jack_port_register (client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    /* set up the ringbuffers */
    input_rb  = jack_ringbuffer_create(80000);
    output_rb = jack_ringbuffer_create(80000);
    
    /* tell the JACK server that we are ready to roll */

    if (jack_activate (client)) {
	fprintf (stderr, "cannot activate client");
	return 1;
    }

    /* connect the ports. Note: you can't do this before
       the client is activated, because we can't allow
       connections to be made to clients that aren't
       running.
       */

    if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == NULL) {
	fprintf(stderr, "Cannot find any physical capture ports\n");
	exit(1);
    }

    if (jack_connect (client, ports[0], jack_port_name (input_port))) {
	fprintf (stderr, "cannot connect input ports\n");
    }

    free (ports);

    if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
	fprintf(stderr, "Cannot find any physical playback ports\n");
	exit(1);
    }

    if (jack_connect (client, jack_port_name (output_port), ports[0])) {
	fprintf (stderr, "cannot connect output ports\n");
    }

    free (ports);

    /* Since this is just a toy, run for a few seconds, then finish */

    sleep (10);
    jack_client_close (client);
    jack_ringbuffer_free(input_rb);
    jack_ringbuffer_free(output_rb);
    exit (0);
}

