#include <iostream>
#include "RtspClientManager.h"

RTSPClientManager::RTSPClientManager(const int decod_thr_num)
{
    decode_thr_num_ = decod_thr_num;
    scheduler_ = BasicTaskScheduler::createNew();
    env_ = BasicUsageEnvironment::createNew(*scheduler_);
    decoder_factory_ = new DecoderFactory;
    decoder_factory_->Initsize(decod_thr_num);
}

RTSPClientManager::~RTSPClientManager()
{
    env_->reclaim(); env_ = NULL;
    delete scheduler_; scheduler_ = NULL;
    delete decoder_factory_;
}

void RTSPClientManager::RunEventLoop()
{
    env_->taskScheduler().doEventLoop(&eventLoopWatchVariable_);
}

void RTSPClientManager::OpenRtsp(const std::__cxx11::string &rtsp_url, const DecoderFactory::DecoderType t, const std::function<void (const AVPixelFormat, const u_char *, const int, const int)> handler)
{
    //一路rtsp一个decoder,保持sps,pps对应不然会解不出来
    std::lock_guard<std::mutex> lock(handler_mtx_);
    if(client_map_.find(rtsp_url) != client_map_.end())return;
    MultiRTSPClient *client = new MultiRTSPClient(*env_,rtsp_url.data(),1,"multi_rtsp_client");
    client->decoder_ = decoder_factory_->MakeDecoder(t);
    client->decoder_->SetFrameCallBack(handler);
    client->sendDescribeCommand(continueAfterDESCRIBE);
    client_map_.insert(std::make_pair(rtsp_url, client));
}

void RTSPClientManager::ShutDownClient(const std::string &rtsp_url)
{
    std::lock_guard<std::mutex> lock(handler_mtx_);
    auto iter = client_map_.find(rtsp_url);
    if(iter == client_map_.end())return;
    shutdownStream(iter->second);
    client_map_.erase(iter);
}
