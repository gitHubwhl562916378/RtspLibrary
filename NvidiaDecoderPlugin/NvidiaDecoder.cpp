#include "NvidiaDecoder.h"

simplelogger::Logger *logger = simplelogger::LoggerFactory::CreateConsoleLogger();
bool isInitsized = false;
int gcurIndex = 0;
std::mutex gmtx;
std::vector<std::pair<CUcontext,std::string>> m_ctxV;
NvidiaDecoder::NvidiaDecoder(TaskManager *t)
{
    taskManager_ = t;
}

NvidiaDecoder::~NvidiaDecoder()
{
    if(m_nvdecod)
    delete m_nvdecod;
}

void NvidiaDecoder::Decode(const AVPacket *pkt)
{
    if(!m_ctxV.size()){
        return;
    }

    taskManager_->GetIOService().post([=]{
        try{
            std::lock_guard<std::mutex> lock(decod_mtx_);
            int nFrameReturned = 0;
            uint8_t **ppFrame;
            m_nvdecod->Decode(pkt->data, pkt->size, &ppFrame, &nFrameReturned);
            if (!nFrameReturned)
                LOG(INFO) << m_nvdecod->GetVideoInfo();

            for (int i = 0; i < nFrameReturned; i++) {
                if (m_nvdecod->GetBitDepth() == 8){
                    frame_buffer_ = ppFrame[i];
                    frame_Hander_(AV_PIX_FMT_NV12,frame_buffer_,m_nvdecod->GetWidth(),m_nvdecod->GetHeight());
                }else{
                    //                    P016ToBgra32((uint8_t *)ppFrame[i], 2 * dec.GetWidth(), (uint8_t *)dpFrame, nPitch, dec.GetWidth(), dec.GetHeight());
                }
            }

            return;
        }catch(std::exception &e){
            std::cout << __func__ << ":" << e.what() << std::endl;
            return;
        }
    });
}

void NvidiaDecoder::SetFrameCallBack(const std::function<void (const AVPixelFormat, const u_char *, const int, const int)> frameHandler)
{
    frame_Hander_ = frameHandler;
}

bool NvidiaDecoder::Initsize(const AVCodecID codec, std::string &error)
{
    gmtx.lock();
    if(!isInitsized){
        ck(cuInit(0));
        int nGpu = 0;
        ck(cuDeviceGetCount(&nGpu));
        for(int i = 0; i < nGpu; i++){
            CUdevice cuDevice = 0;
            ck(cuDeviceGet(&cuDevice, i));
            char szDeviceName[80];
            ck(cuDeviceGetName(szDeviceName, sizeof(szDeviceName), cuDevice));
            CUcontext cuContext = NULL;
            ck(cuCtxCreate(&cuContext, CU_CTX_SCHED_BLOCKING_SYNC, cuDevice));
            LOG(INFO) << "Find Gpu: " << szDeviceName << std::endl;

            CUVIDDECODECAPS videoDecodeCaps = {};
            videoDecodeCaps.eCodecType = cudaVideoCodec_H264;
            videoDecodeCaps.eChromaFormat = cudaVideoChromaFormat_420;
            videoDecodeCaps.nBitDepthMinus8 = 0;
            CUresult resCode;
            if ((resCode = cuvidGetDecoderCaps(&videoDecodeCaps)) == CUDA_SUCCESS){
                LOG(INFO) << "cuvid Decoder Caps nMaxWidth " << videoDecodeCaps.nMaxWidth << " nMaxHeigth " << videoDecodeCaps.nMaxHeight << std::endl;
                if(videoDecodeCaps.nMaxWidth >= 1920 && videoDecodeCaps.nMaxHeight >= 1080){
                    m_ctxV.push_back({cuContext,szDeviceName});
                }
            }else{
                LOG(INFO) << "cuvidGetDecoderCaps failed, Code: " << resCode << std::endl;
            }
        }
        isInitsized = true;
        LOG(INFO) << "nvidia decoder initsized end " << isInitsized << " ptr is " << &isInitsized << std::endl;
    }
    gmtx.unlock();

    if(m_ctxV.empty()){
        error = "no context for this width and height";
        return false;
    }


    std::pair<CUcontext,std::string> v = m_ctxV.at(gcurIndex++ % m_ctxV.size());
    std::cout << "Use Contex in" << v.second << " ctx size " << m_ctxV.size() << std::endl;
    m_nvdecod = new NvDecoder(v.first, false, FFmpeg2NvCodecId(codec), nullptr);
    return true;
}

extern "C"
Decoder* createDecoder(TaskManager* t)
{
    return new NvidiaDecoder(t);
}
