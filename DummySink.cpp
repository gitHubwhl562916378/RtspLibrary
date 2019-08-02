extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include "H264VideoRTPSource.hh"
#include "DummySink.h"
#include "Queue.h"

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId, const Decoder *d):
    MediaSink(env),
    fSubsession(subsession),
    decoder_(d)
{
    fStreamId = ::strDup(streamId);
    if(fStreamId[::strlen(fStreamId) - 1] == '/'){
        fStreamId[::strlen(fStreamId) - 1] = '\0';
    }
    fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];

    p_nalu_tail = new char[1024*1024];
    nalu_buffer = p_nalu_tail;
}

DummySink::~DummySink()
{
    delete[] fReceiveBuffer;
    delete[] fStreamId;
    delete[] nalu_buffer;
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds)
{
    DummySink* sink = (DummySink*)clientData;
    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);

}
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <sstream>
void Base64Decode(const std::string &input, std::string &output)
{
    typedef boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<std::string::const_iterator>, 8, 6> Base64DecodeIterator;
    std::stringstream result;
    try {
        std::copy( Base64DecodeIterator( input.begin() ), Base64DecodeIterator( input.end() ), std::ostream_iterator<char>( result ) );
    } catch ( ... ) {
        return false;
    }
    output = result.str();
    return output.empty() == false;
}

#include <iostream>
void DummySink::afterGettingFrame(unsigned frameSize,unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds)
{
    // We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
    if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
    envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
    if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
    char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
    sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
    envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr << "\tprotocol:" << fSubsession.protocolName();
    if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
        envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
    }
#ifdef DEBUG_PRINT_NPT
    envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
    envir() << "\n";
#endif

    u_char const start_code[4]{0x00,0x00,0x00,0x01};
    if(!::strcmp(fSubsession.codecName(),"H264"))
    {
        if (isFirstFrame_)            // 仅每次播放的第一次进入执行本段代码
        {    // 对视频数据的SPS,PPS进行补偿
            std::string msg;
            decoder_->Initsize(AV_CODEC_ID_H264, msg);
            unsigned numSPropRecords;
            SPropRecord* sPropRecords = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), numSPropRecords);
//            SPropRecord &sps = sPropRecords[0];
//            SPropRecord &pps = sPropRecords[1];
//            std::cout << std::hex;
//            for(int i = 0; i < sps.sPropLength; i++){
//                std::cout << (int)sps.sPropBytes[i] << " ";
//            }
//            std::cout << std::endl;

            // spydroid v6.8 or spydroid v9.1.
            for (unsigned i = 0; i < numSPropRecords; ++i)
            {
                memcpy(p_nalu_tail, start_code, sizeof(start_code));
                p_nalu_tail += sizeof(start_code);
                memcpy(p_nalu_tail, sPropRecords[i].sPropBytes, sPropRecords[i].sPropLength);
                p_nalu_tail += sPropRecords[i].sPropLength;
                delete[] sPropRecords[i].sPropBytes;
            }
            isFirstFrame_ = false; // 标记SPS,PPS已经完成补偿

            memcpy(p_nalu_tail, start_code, sizeof(start_code));
            p_nalu_tail += sizeof(start_code);
            memcpy(p_nalu_tail, fReceiveBuffer, frameSize);
            p_nalu_tail += frameSize;
        }
        else
        {
            if(presentationTime.tv_sec == pre_time_stamp.tv_sec && presentationTime.tv_usec == pre_time_stamp.tv_usec)
            {
                memcpy(p_nalu_tail, start_code, sizeof(start_code));
                p_nalu_tail += sizeof(start_code);
                memcpy(p_nalu_tail, fReceiveBuffer, frameSize);
                p_nalu_tail += frameSize;
            }
            else
            {
                if(p_nalu_tail != nalu_buffer)
                {
                    AVPacket *paket = new AVPacket;
                    av_new_packet(paket, p_nalu_tail - nalu_buffer);
                    memcpy(paket->data , nalu_buffer, p_nalu_tail - nalu_buffer);

                    decoder_->Decode(paket);
                }
                p_nalu_tail = nalu_buffer;
                memcpy(p_nalu_tail, start_code, sizeof(start_code));
                p_nalu_tail += sizeof(start_code);
                memcpy(p_nalu_tail, fReceiveBuffer, frameSize);
                p_nalu_tail += frameSize;
            }
        }
        pre_time_stamp = presentationTime;
    }

    // Then continue, to request the next frame of data:
    continuePlaying();
}

Boolean DummySink::continuePlaying()
{
    if (fSource == NULL) return False; // sanity check (should not happen)

    // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                          afterGettingFrame, this,
                          onSourceClosure, this);
    return True;
}
