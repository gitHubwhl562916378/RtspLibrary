#pragma once

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include <memory>
#include <functional>

class DecoderFactory;
class Decoder
{
public:
    explicit Decoder() = default;
    virtual ~Decoder(){}
    /***
     * @codec   视频编码方式
     * @width   视频宽度
     * @height  视频高度
     * @error   错误信息
    ***/
    virtual bool Initsize(const AVCodecID codec,std::string &error) = 0;
    /***
     * @format 图片格式
     * @data   数据
     * @width  图片宽度
     * @height 图片高度
     ***/
    virtual void SetFrameCallBack(const std::function<void(const AVPixelFormat format, const u_char *data, const int width, const int height)> frameHandler) = 0;
    /***
     * @pkt 数据包，带pps和sps
     * @frameHandler 解码成功后的回调函数，
     *     @format 图上格式
     *     @data 数据地址
     *     @width 图片宽度
     *     @height 图片高度
     * @error 错误信息
    ***/
    virtual void Decode(const AVPacket *pkt) = 0;
};
