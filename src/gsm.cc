#include "gsm.h"
#include <iax/iax-client.h>


int GSMCoder::encode(jack_ringbuffer_t *in)
{ 
    jack_ringbuffer_t *out = encoded_rb;
    gsm_signal src[160];
    gsm_frame dst;
    int n = 0;
    while (jack_ringbuffer_read_space(in) >= sizeof(src) && 
            jack_ringbuffer_write_space(out) >= sizeof(dst))
    {
        jack_ringbuffer_read(in,(char*)src,sizeof(src));
        gsm_encode(handle,src,dst);
        jack_ringbuffer_write(out,(char*)dst,sizeof(dst));
        n += sizeof(dst);
    }
    return n;
}

int GSMCoder::decode(jack_ringbuffer_t *in)
{
    jack_ringbuffer_t *out = decoded_rb;
    gsm_frame src;
    gsm_signal dst[160];
    int n = 0;
    while (jack_ringbuffer_read_space(in) >= sizeof(src) && 
            jack_ringbuffer_write_space(out) >= sizeof(dst))
    {
        jack_ringbuffer_read(in,(char*)src,sizeof(src));
        gsm_decode(handle,src,dst);
        jack_ringbuffer_write(out,(char*)dst,sizeof(dst));
        n += sizeof(dst);
    }
    return n;
}
