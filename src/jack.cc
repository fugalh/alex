/* Based on simple_client.c from the JACK distribution */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "jack.h"
#include "threads.h"

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

int Jack::jack_process(jack_nframes_t nframes, void *arg)
{
    int ret;
    int n = sizeof (jack_default_audio_sample_t) * nframes;
    jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) 
	jack_port_get_buffer (input_port, nframes);
    jack_default_audio_sample_t *out = (jack_default_audio_sample_t *) 
	jack_port_get_buffer (output_port, nframes);

    if (!off_hook)
    {
        // enjoy the silence
        memset(in,0,n);
        memset(out,0,n);
        return 0;
    }

    // input
    ret = jack_ringbuffer_write(input_rb, (char*)in, n);
    if (ret < n) { /* somebody do something! */ }
    sem_post(event_sem);

    // output
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
