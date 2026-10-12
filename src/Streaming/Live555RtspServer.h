#ifndef LIVE555_RTSP_SERVER
#define LIVE555_RTSP_SERVER

#include <thread>
#include <atomic>
#include <iostream>
#include <UsageEnvironment.hh>
#include <BasicUsageEnvironment.hh>
#include <GenericMediaServer.hh>
#include <RTSPServer.hh>
#include "H264LiveSubsession.h"
#include "AACLiveSubsession.h"
#include "Queue.h"
#include "Define.h"

class Live555RtspServer
{
public:
    Live555RtspServer();
    ~Live555RtspServer();

    void Config(const Live555RtspConfig& Config);
    bool Start(uint16_t port = 8554, const std::string& streamName = "live");
    void Stop();

private:
    std::shared_ptr<Queue<UniquePacketPtr>> m_videoQueue;
    std::shared_ptr<Queue<UniquePacketPtr>> m_audioQueue; 
    std::jthread m_thread;
    std::atomic<bool> m_running{false};
};

#endif // LIVE555_RTSP_SERVER