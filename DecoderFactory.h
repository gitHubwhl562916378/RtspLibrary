#pragma once

#include "FfmpegDecoder.h"

class DecoderFactory
{
public:
    enum Type{
        Ffmpeg,
        Nvidia
    };
    Decoder* MakeDecoder(Type t = Ffmpeg){
        Decoder *d = nullptr;
        switch (t) {
        case Ffmpeg:
            d = new FfmpegDecoder;
            break;
        case Nvidia:
            break;
        }
        return d;
    }
};
