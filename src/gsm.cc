#include "gsm.h"
#include <iax/iax-client.h>


void GSM::encode(short *src, int *srclen, char *dst, int *dstlen)
{ 
    gsm_signal *sbp = src;
    gsm_signal *sep = src + *srclen;
    gsm_byte   *dbp = (gsm_byte*)dst;
    gsm_byte   *dep = (gsm_byte*)dst + *dstlen;

    while ((sep-sbp) >= 160 && (dep-dbp) >= 33)
    {
        gsm_encode(handle, sbp, dbp);
        sbp += 160;
        dbp += 33;
    }
    *srclen = sbp - src;
    *dstlen = dbp - (gsm_byte*)dst;
}

void GSM::decode(char *src, int *srclen, short *dst, int *dstlen)
{ 
    gsm_byte   *sbp = (gsm_byte*)src;
    gsm_byte   *sep = (gsm_byte*)src + *srclen;
    gsm_signal *dbp = dst;
    gsm_signal *dep = dst + *dstlen;

    while ((sep-sbp) >= 33 && (dep-dbp) >= 160)
    {
        gsm_decode(handle, sbp, dbp);
        sbp += 33;
        dbp += 160;
    }
    *srclen = sbp - (gsm_byte*)src;
    *dstlen = dbp - dst;
}
