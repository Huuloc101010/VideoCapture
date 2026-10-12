#ifndef CUSTOM_PACKET_SOURCE
#define CUSTOM_PACKET_SOURCE

#include <memory>
#include <liveMedia.hh>
#include "Queue.h"
#include "Define.h"

class CustomPacketSource : public FramedSource {
public:
    static CustomPacketSource* createNew(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue);
protected:
    CustomPacketSource(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue);
    void doGetNextFrame() override;
    virtual unsigned maxFrameSize() const override;
private:
    static void DeferredGetNextFrame(void* clientData);
    void DeliverFrame(const UniquePacketPtr& Packet);
    std::shared_ptr<Queue<UniquePacketPtr>> m_Queue;
};

#endif // CUSTOM_PACKET_SOURCE