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
    m_Config = Config;
}

bool Live555RtspServer::Start(uint16_t port, const std::string& streamName)
{
    m_running = 0;
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
        sms->addSubsession(H264LiveSubsession::createNew(*env, m_videoQueue, m_Config.VideoTimeBase));

        // Attach AAC Audio Track (48kHz, 2 Channels)
        sms->addSubsession(AACLiveSubsession::createNew(*env, m_audioQueue, 48000, 2, m_Config.AudioTimeBase));

        rtspServer->addServerMediaSession(sms);
        char* Prefix = rtspServer->rtspURLPrefix();
        std::cout << "\n[RTSP Server] Streaming ready at: " << Prefix << streamName << "\n\n";
        delete[] Prefix;

        // Event Loop
        std::cout << "Doing loop" << std::endl;
            if(scheduler)
            {
                scheduler->doEventLoop(&m_running);
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
    m_running = 1;
    LOGI("m_running = 1");
    if(m_thread.joinable())
    {
        m_thread.request_stop();
        m_thread.join();
    }
    LOGI("Success");
}