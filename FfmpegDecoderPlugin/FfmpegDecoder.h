#pragma once

#include <atomic>
#include <mutex>
#include "Decoder.h"
#include "TaskManager.h"

class FfmpegDecoder : public Decoder
{
public:
    explicit FfmpegDecoder(TaskManager* t);
    ~FfmpegDecoder();

    bool Initsize(const AVCodecID codec, std::string &msg) override;
    virtual void SetFrameCallBack(const std::function<void(const AVPixelFormat format, const u_char *data, const int width, const int height)> frameHandler) override;
    void Decode(const AVPacket *pkt) override;

private:
    TaskManager *taskManager_;
    static std::atomic_bool ffmpeg_inited_;
    AVFrame *decoded_frame_;
    AVCodec *codec_;
    AVCodecContext *codec_context_;

    std::mutex decod_mtx_;
    u_char* frame_buffer_;
    int video_width_;
    int video_height_;
    bool is_first_frame_ = true;
    std::function<void(const AVPixelFormat format, const u_char *data, const int width, const int height)> frame_Hander_;
};
