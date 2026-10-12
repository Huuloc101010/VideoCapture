#ifndef STREAMING_SERVER
#define STREAMING_SERVER

#include <memory>
#include "Queue.h"
#include "Define.h"
#include "Live555RtspServer.h"

class StreamingServer
{
public:
    StreamingServer();
    ~StreamingServer() = default;
    bool Config(const StreamingServerConfig& Config);
    bool Start();
    void Stop();

private:
    std::shared_ptr<Queue<UniquePacketPtr>> m_VideoQueue;
    std::shared_ptr<Queue<UniquePacketPtr>> m_AudioQueue;
    Live555RtspServer     m_RtspServer;
    StreamingServerConfig m_Config;
};

#endif // STREAMING_SERVER