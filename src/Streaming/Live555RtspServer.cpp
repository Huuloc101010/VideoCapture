#include "Live555RtspServer.h"

Live555RtspServer::Live555RtspServer()
{

};
Live555RtspServer::~Live555RtspServer()
{
    Stop();
}

void Live555RtspServer::Config(const Live555RtspConfig& Config)
{
    m_videoQueue = Config.VideoQueue;
    m_audioQueue = Config.AudioQueue;
}

bool Live555RtspServer::Start(uint16_t port, const std::string& streamName)
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

        // Create new Sesson: rtsp://<IP>:8554/live
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

void Live555RtspServer::Stop()
{
    m_running = false;
    if(m_thread.joinable())
    {
        m_thread.request_stop();
    }
}