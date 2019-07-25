#include "FfmpegDecoder.h"

std::atomic_bool FfmpegDecoder::ffmpeg_inited_=false;
FfmpegDecoder::~FfmpegDecoder()
{
    if(frame_buffer_){
        delete[] frame_buffer_;
    }
    if(decoded_frame_){
        av_frame_free(&decoded_frame_);
    }
    if(codec_context_){
        avcodec_free_context(&codec_context_);
    }
}

bool FfmpegDecoder::Initsize(const AVCodecID codec, const int width, const int height, std::string &msg)
{
    if(!ffmpeg_inited_){
        avcodec_register_all();
        ffmpeg_inited_ = true;
    }

    decoded_frame_ = av_frame_alloc();
    codec_ = avcodec_find_decoder(codec);
    if(!codec_){
        msg.assign("codec not find");
        return false;
    }
    codec_context_ = avcodec_alloc_context3(codec_);
    if(avcodec_open2(codec_context_, codec_, nullptr) < 0){
        msg.assign("could not open codec");
        return false;
    }

    video_width_ = codec_context_->width;
    video_height_ = codec_context_->height;
    return true;
}

bool FfmpegDecoder::Decode(const AVPacket *pkt, const std::function<void(const AVPixelFormat, const u_char *, const int, const int)> frameHandler, std::string &msg)
{
    int frameFinished = 0;
    int resCode = avcodec_decode_video2(codec_context_, decoded_frame_, &frameFinished, pkt);
    if(!resCode){
        msg.assign("decode pkg error: " + std::to_string(resCode));
        av_free_packet(pkt);
        delete pkt;
        return false;
    }

    if(!frameFinished){
        av_free_packet(pkt);
        delete pkt;
        return true;
    }

    if(is_first_frame_){
        int numBytes = avpicture_get_size(codec_context_->pix_fmt,video_width_,video_height_);
        frame_buffer_ = (u_char*)av_malloc(numBytes * sizeof(uint8_t));
        is_first_frame_ = false;
    }

    int bytes = 0; //yuv data有3块内存分别拷，nv12只有2块内存分别拷
    for(int i = 0; i <video_height_; i++){ //将y分量拷贝
        ::memcpy(frame_buffer_ + bytes,decoded_frame_->data[0] + decoded_frame_->linesize[0] * i, video_width_);
        bytes += video_width_;
    }
    int uv = video_height_ >> 1;
    for(int i = 0; i < uv; i++){ //将u分量拷贝
        ::memcpy(frame_buffer_ + bytes,decoded_frame_->data[1] + decoded_frame_->linesize[1] * i, video_width_ >> 1);
        bytes += video_width_ >> 1;
    }
    for(int i = 0; i < uv; i++){ //将v分量拷贝
        ::memcpy(frame_buffer_ + bytes,decoded_frame_->data[2] + decoded_frame_->linesize[2] * i, video_width_ >> 1);
        bytes += video_width_ >> 1;
    }
    frameHandler(codec_context_->pix_fmt, frame_buffer_, video_width_, video_height_);
    av_free_packet(pkt);
    delete pkt;
    return true;
}
