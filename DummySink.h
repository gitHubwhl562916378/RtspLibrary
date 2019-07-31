#pragma once
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include "MediaSink.hh"
#include "MediaSession.hh"
#include "Boolean.hh"
#include "UsageEnvironment.hh"
#include "Decoder.h"
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000
#define DEBUG_PRINT_EACH_RECEIVED_FRAME

class DummySink: public MediaSink
{
public:
    DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId, const Decoder *d);
    virtual ~DummySink();

private:
    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);
    void afterGettingFrame(unsigned frameSize,unsigned numTruncatedBytes,
                            struct timeval presentationTime,
                            unsigned durationInMicroseconds);
    
    Boolean continuePlaying();

    u_int8_t* fReceiveBuffer = nullptr;
    bool isFirstFrame_ = true;
    MediaSubsession& fSubsession;
    char* fStreamId;

    Decoder *decoder_;
    struct timeval pre_time_stamp = {0,0};
    char *p_nalu_tail = nullptr;
    char *nalu_buffer = nullptr;
};
