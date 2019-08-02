#pragma once
#include <map>
#include <functional>
#include <boost/asio/io_service.hpp>
#include <thread>
extern "C"
{
#include "libavformat/avformat.h"
}
#include "MultiRTSPClient.h"
#include "Queue.h"
#include "DecoderFactory.h"

class RTSPClientManager
{
public:
    explicit RTSPClientManager(const int decod_thr_num);
    virtual ~RTSPClientManager();

    void RunEventLoop();
    void OpenRtsp(const std::string &rtsp_url, const DecoderFactory::DecoderType t, const std::function<void(const AVPixelFormat format, const u_char *data, const int width, const int height)> handler);
    void ShutDownClient(const std::string &rtsp_url);

private:
    volatile char eventLoopWatchVariable_ = 0;
    TaskScheduler *scheduler_;
    UsageEnvironment *env_;
    std::mutex handler_mtx_;

    int decode_thr_num_;
    volatile bool thread_pool_quit_ = false;
    DecoderFactory *decoder_factory_;
    std::map<std::string,RTSPClient*> client_map_;
    std::vector<std::thread> decod_threads_vec_;
};
