#pragma once

#include <atomic>
#include "Decoder.h"

class FfmpegDecoder : public Decoder
{
public:
    explicit FfmpegDecoder() = default;
    ~FfmpegDecoder();

    bool Initsize(const AVCodecID codec, const int width, const int height, std::string &msg) override;
    bool Decode(const AVPacket *pkt, const std::function<void(const AVPixelFormat format, const u_char *data, const int width, const int height)> frameHandler, std::string &msg) override;

private:
    static std::atomic_bool ffmpeg_inited_;
    AVFrame *decoded_frame_;
    AVCodec *codec_;
    AVCodecContext *codec_context_;

    u_char* frame_buffer_;
    int video_width_;
    int video_height_;
    bool is_first_frame_ = true;
};
