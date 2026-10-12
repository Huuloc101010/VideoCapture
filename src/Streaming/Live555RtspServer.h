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

class Live555RtspServer {
public:
    Live555RtspServer() {};
    ~Live555RtspServer() { Stop(); }

    void Config(const Live555RtspConfig& Config)
    {
        m_videoQueue = Config.VideoQueue;
        m_audioQueue = Config.AudioQueue;
        if(m_videoQueue == nullptr || m_audioQueue == nullptr)
        {
            LOGE("QUeue is nullpt");
            LOGE("QUeue is nullpt");
            LOGE("QUeue is nullpt");
            LOGE("QUeue is nullpt");
            LOGE("QUeue is nullpt");
            LOGE("QUeue is nullpt");
        }
    }

    bool Start(uint16_t port = 8554, const std::string& streamName = "live")
    {
        m_running = true;

        m_thread = std::jthread([this, port, streamName](std::stop_token st)
        {
            TaskScheduler* scheduler = BasicTaskScheduler::createNew();
            UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

            UserAuthenticationDatabase* authDB = nullptr;
            RTSPServer* rtspServer = RTSPServer::createNew(*env, port, authDB);

            if(rtspServer == nullptr)
            {
                std::cerr << "Failed to create RTSP Server on port " << port << std::endl;
                env->reclaim();
                delete scheduler;
                return;
            }

            // Tạo Session mới: rtsp://<IP>:8554/live
            ServerMediaSession* sms = ServerMediaSession::createNew(*env, streamName.c_str(), "Live Stream", "FFmpeg C++ Stream");

            // Attach H.264 Video Track
            sms->addSubsession(H264LiveSubsession::createNew(*env, m_videoQueue));

            // Attach AAC Audio Track (48kHz, 2 Channels)
            sms->addSubsession(AACLiveSubsession::createNew(*env, m_audioQueue, 48000, 2));

            rtspServer->addServerMediaSession(sms);

            std::cout << "\n[RTSP Server] Streaming ready at: " << rtspServer->rtspURLPrefix() << streamName << "\n\n";

            // Event Loop
            std::atomic<char> stopFlag = 0;
            while (!st.stop_requested() && m_running)
            {
                std::cout << "Doing loop" << std::endl;
                scheduler->doEventLoop(&stopFlag);
            }
            std::cout << "Do looop success" << std::endl;
            Medium::close(rtspServer);
            env->reclaim();
            delete scheduler;
        });

        return true;
    }

    void Stop() {
        m_running = false;
        if(m_thread.joinable())
        {
            m_thread.request_stop();
        }
    }

private:
    std::shared_ptr<Queue<UniquePacketPtr>> m_videoQueue;
    std::shared_ptr<Queue<UniquePacketPtr>> m_audioQueue; 
    std::jthread m_thread;
    std::atomic<bool> m_running{false};
};

#endif // LIVE555_RTSP_SERVER