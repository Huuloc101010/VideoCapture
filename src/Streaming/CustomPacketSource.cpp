#include "CustomPacketSource.h"

CustomPacketSource::CustomPacketSource(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue)
    : FramedSource(env), m_Queue(queue)
{

}

CustomPacketSource* CustomPacketSource::createNew(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue)
{
    return new CustomPacketSource(env, queue);
}

void CustomPacketSource::doGetNextFrame()
{
    if(m_Queue == nullptr)
    {
        return;
    }
    if(m_Queue->Size())
    {
        DeliverFrame(std::move(m_Queue->Pop()));
    }
    else
    {
        // If queue empty -> Register 1 stask schedule for checking after that
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            10000, // 10,000 microseconds = 10ms
            (TaskFunc*)DeferredGetNextFrame, 
            this
        );
    }
}

unsigned CustomPacketSource::maxFrameSize() const
{
    return 10000;
}
void CustomPacketSource::DeferredGetNextFrame(void* clientData)
{
    static_cast<CustomPacketSource*>(clientData)->doGetNextFrame();
}

void CustomPacketSource::DeliverFrame(const UniquePacketPtr& Packet)
{
    if (Packet == nullptr)
    {
        handleClosure();
        return;
    }

    // Check buffer overflow
    fFrameSize = std::min(static_cast<unsigned>(Packet->size), fMaxSize);
    memcpy(fTo, Packet->data, fFrameSize);
    if(Packet->size > fMaxSize)
    {
        fNumTruncatedBytes = Packet->size - fMaxSize;
        LOGW("Truncated Packet->size {} - fMaxSize {}", Packet->size , fMaxSize);
    }
    else
    {
        fNumTruncatedBytes = 0;
    }

    // update timestamp for RTP Packet
    timeval tv;
    tv.tv_sec = static_cast<long>(Packet->pts);
    tv.tv_usec = static_cast<long>((Packet->pts - tv.tv_sec) * 1e6);
    fPresentationTime = tv;

    // Notify for Live555 buffer ready
    FramedSource::afterGetting(this);
}