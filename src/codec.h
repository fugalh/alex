#ifndef CODEC_H
#define CODEC_H

class Codec
{
    public:
        virtual void encode(short *src, int *srclen, char *dst, int *dstlen) = 0;
        virtual void decode(char *src, int *srclen, short *dst, int *dstlen) = 0;
};

#endif
