#pragma once

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include <memory>
#include <functional>

class Decoder
{
public:
    explicit Decoder() = default;
    virtual ~Decoder(){}
    /***
     * @codec   视频编码方式
    ***/
    virtual bool Initsize(const AVCodecID codec, const int width, const int height, std::string &error) = 0;
    /***
     * @pkt 数据包，带pps和sps
     * @frameHandler 解码成功后的回调函数，
     *     @format 图上格式
     *     @data 数据地址
     *     @width 图片宽度
     *     @height 图片高度
    ***/
    virtual bool Decode(const AVPacket *pkt, const std::function<void(const AVPixelFormat format, const u_char *data, const int width, const int height)> frameHandler, std::string &msg) = 0;
};
