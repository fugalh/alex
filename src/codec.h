#ifndef CODEC_H
#define CODEC_H

class Codec
{
    public:
        virtual int encode(short*,int*,void*,int*) = 0;
        virtual int decode(void*,int*,short*,int*) = 0;
};

class GSM : public Codec
{
    public:
        int encode(short*,int*,void*,int*);
        int decode(void*,int*,short*,int*);
};

#endif
