#ifndef ALEX_GSM_H
#define ALEX_GSM_H

#include "codec.h"
#include <gsm.h>

class GSM : public Codec
{
    public:
        GSM() { framesize = 160; handle = gsm_create(); }
        virtual ~GSM() { gsm_destroy(handle); }
        void encode(short *src, int *srclen, char *dst, int *dstlen);
        void decode(char *src, int *srclen, short *dst, int *dstlen);

    protected:
        gsm handle;
};

#endif
