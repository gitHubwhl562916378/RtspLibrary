#pragma once
#include <thread>
#include "FfmpegDecoder.h"
#include "TaskManager.h"

class DecoderFactory
{
public:
    enum Type{
        Ffmpeg,
        Nvidia
    };
    explicit DecoderFactory() = default;
    virtual ~DecoderFactory(){}
    void Initsize(int thr_num){
        taskManager_ = new TaskManager(thr_num);
    }
    Decoder* MakeDecoder(Type t = Ffmpeg){
        Decoder *d = nullptr;
        switch (t) {
        case Ffmpeg:
            d = new FfmpegDecoder(taskManager_);
            break;
        case Nvidia:
            break;
        }
        return d;
    }

private:
    TaskManager *taskManager_;
};
