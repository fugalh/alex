#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "audio.h"
#include "codec.h"
#include <vector>

using std::vector;

class Protocol
{
    public:
        virtual ~Protocol() {}
        Audio *audio;
        vector<int> codecs;
        /// Lazy lookup
        virtual Codec *get_codec(int format) = 0;
};

#endif
