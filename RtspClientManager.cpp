#include <iostream>
#include "RtspClientManager.h"

RTSPClientManager::RTSPClientManager(const int decod_thr_num)
{
    decode_thr_num_ = decod_thr_num;
    scheduler_ = BasicTaskScheduler::createNew();
    env_ = BasicUsageEnvironment::createNew(*scheduler_);
    decoder_factory_ = new DecoderFactory;
}

RTSPClientManager::~RTSPClientManager()
{
    env_->reclaim(); env_ = NULL;
    delete scheduler_; scheduler_ = NULL;
    delete decoder_factory_;
}

void RTSPClientManager::RunEventLoop()
{
    for(int i = 0; i < decode_thr_num_; i++)
    {
        decod_threads_vec_.push_back(std::thread([this]{
            Decoder *decoder = decoder_factory_->MakeDecoder();
            std::string error;
            if(!decoder->Initsize(AV_CODEC_ID_H264,error))
            {
                std::cout << error << std::endl;
                return;
            }

            while (!thread_pool_quit_) {
                std::pair<std::string, AVPacket*> pkt = pkt_queue_.Pop();
                std::function<void (const AVPixelFormat, const u_char *, const int, const int)> handler_func = nullptr;
                {
                    std::lock_guard<std::mutex> lock(handler_mtx_);
                    if(auto iter = handler_map_.find(pkt.first); iter != handler_map_.end()){
                        handler_func = iter->second;
                    }
                }
                if(handler_func){
                    if(!decoder->Decode(pkt.second, handler_func, error))
                    {
                        std::cout << error << std::endl;
                        continue;
                    }
                }
            }
        }));
    }
    env_->taskScheduler().doEventLoop(&eventLoopWatchVariable_);
}

void RTSPClientManager::OpenRtsp(const std::__cxx11::string &rtsp_url, const std::function<void (const AVPixelFormat, const u_char *, const int, const int)> handler)
{
    //一路rtsp一个decoder,保持sps,pps对应不然会解不出来
    {
        std::lock_guard<std::mutex> lock(handler_mtx_);
        if(handler_map_.find(rtsp_url) != handler_map_.end())return;
        handler_map_.insert(std::make_pair(rtsp_url, handler));
    }
    MultiRTSPClient *client = new MultiRTSPClient(*env_,rtsp_url.data(),pkt_queue_,1,"multi_rtsp_client");
    client->sendDescribeCommand(continueAfterDESCRIBE);
}
