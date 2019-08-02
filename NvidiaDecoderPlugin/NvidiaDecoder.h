#ifndef NVIDIADECODER_H
#define NVIDIADECODER_H

#include <vector>
#include "Utils/FFmpegDemuxer.h"
#include "NvDecoder.h"
#include "Decoder.h"
#include "TaskManager.h"

class NvidiaDecoder : public Decoder
{
public:
    explicit NvidiaDecoder(TaskManager* t);
    ~NvidiaDecoder();

    bool Initsize(const AVCodecID codec,std::string &error) override;

    void SetFrameCallBack(const std::function<void(const AVPixelFormat format, const u_char *data, const int width, const int height)> frameHandler) override;

    void Decode(const AVPacket *pkt) override;

private:
    NvDecoder *m_nvdecod{nullptr};

    TaskManager *taskManager_;
    std::mutex decod_mtx_;
    u_char* frame_buffer_;
    int video_width_;
    int video_height_;
    bool is_first_frame_ = true;
    std::function<void(const AVPixelFormat format, const u_char *data, const int width, const int height)> frame_Hander_;
};

#endif // NVIDIADECODER_H
