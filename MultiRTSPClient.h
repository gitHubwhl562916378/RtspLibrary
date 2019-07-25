#pragma once

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "Queue.h"
#define REQUEST_STREAMING_OVER_TCP True

class StreamClientState {
public:
  StreamClientState():iter(nullptr),session(nullptr),subsession(nullptr),
  streamTimerTask(nullptr),duration(0.0){}

  virtual ~StreamClientState(){
      delete iter;
      if(session != nullptr){
          UsageEnvironment& env = session->envir();

          env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
          Medium::close(session);
      }
  }

public:
  MediaSubsessionIterator* iter;
  MediaSession* session;
  MediaSubsession* subsession;
  TaskToken streamTimerTask;
  double duration;
};

 class MultiRTSPClient : public RTSPClient
 {
 public:
     MultiRTSPClient(UsageEnvironment& env, char const* rtspURL, Queue<std::pair<std::string,AVPacket*>> &pkt_queu, int verbosityLevel = 0,
     char const* applicationName = nullptr, portNumBits tunnelOverHTTPPortNum = 0);
     virtual ~MultiRTSPClient();

     StreamClientState scs;
     Queue<std::pair<std::string,AVPacket*>> &pkt_queue_;
 };

 // RTSP 'response handlers':
 void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
 void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
 void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

 // Other event handler functions:
 void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
 void subsessionByeHandler(void* clientData, char const* reason);
   // called when a RTCP "BYE" is received for a subsession
 void streamTimerHandler(void* clientData);
   // called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

 // Used to iterate through each stream's 'subsessions', setting up each one:
 void setupNextSubsession(RTSPClient* rtspClient);

 // Used to shut down and close a stream (including its "RTSPClient" object):
 void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);
