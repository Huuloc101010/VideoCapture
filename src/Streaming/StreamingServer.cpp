#include "StreamingServer.h"

StreamingServer::StreamingServer()
{
    
}

bool StreamingServer::Config(const StreamingServerConfig& Config)
{
    m_Config = Config;
    m_VideoQueue = Config.VideoQueue;
    m_AudioQueue = Config.AudioQueue;
    m_RtspServer.Config({m_VideoQueue, m_AudioQueue});
    return true;
}

bool StreamingServer::Start()
{
    return m_RtspServer.Start(m_Config.Port, m_Config.Path);
}

void StreamingServer::Stop()
{
    LOGI("Received stop");
    m_RtspServer.Stop();
}