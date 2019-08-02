#pragma once
#include <dlfcn.h>
#include <thread>
#include "Decoder.h"
#include "TaskManager.h"

Decoder *loadPlugin(const char *path, TaskManager*t)
{
    void *handle = dlopen(path,RTLD_LAZY);
    if(!handle){
        std::cout << dlerror() << std::endl;
        return nullptr;
    }

    dlerror();
    typedef Decoder*(*CreatePlugin)(TaskManager*);
    CreatePlugin createFun = dlsym(handle,"createDecoder");

    char *error = nullptr;
    if((error = dlerror()) != nullptr){
        std::cout << error << std::endl;
        return nullptr;
    }
    return createFun(t);
}

class DecoderFactory
{
public:
    enum DecoderType{
        Ffmpeg,
        Nvidia
    };
    explicit DecoderFactory() = default;
    virtual ~DecoderFactory(){}
    void Initsize(int thr_num){
        taskManager_ = new TaskManager(thr_num);
    }
    Decoder* MakeDecoder(DecoderType t = Ffmpeg){
        Decoder *d = nullptr;
        switch (t) {
        case Ffmpeg:
            d = loadPlugin("libFfmpegDecoderPlugin.so",taskManager_);
            break;
        case Nvidia:
            d = loadPlugin("libNvidiaDecoderPlugin.so",taskManager_);
            break;
        }
        return d;
    }

private:
    TaskManager *taskManager_;
};
