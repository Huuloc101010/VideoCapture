#ifndef CUSTOM_PACKET_SOURCE
#define CUSTOM_PACKET_SOURCE

#include <memory>
#include <liveMedia.hh>
#include "Queue.h"
#include "Define.h"

class CustomPacketSource : public FramedSource {
public:
    static CustomPacketSource* createNew(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue)
    {
        return new CustomPacketSource(env, queue);
    }

protected:
    CustomPacketSource(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue)
        : FramedSource(env), m_Queue(queue) {}
    void doGetNextFrame() override
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

    virtual unsigned maxFrameSize() const override
    {
        return 10000;
    }
private:
    static void DeferredGetNextFrame(void* clientData)
    {
        static_cast<CustomPacketSource*>(clientData)->doGetNextFrame();
    }

    void DeliverFrame(const UniquePacketPtr& Packet)
    {
        if (Packet == nullptr)
        {
            handleClosure();
            return;
        }
        //PrintH264NALU(Packet.get());

        // Kiểm tra tránh tràn buffer cấp phát bởi Live555 (fMaxSize)
        fFrameSize = std::min(static_cast<unsigned>(Packet->size), fMaxSize);
        memcpy(fTo, Packet->data, fFrameSize);
        if(Packet->size > fMaxSize)
        {
            fNumTruncatedBytes = Packet->size - fMaxSize;
            LOGW("Truncated Packet->size {} - fMaxSize {}", Packet->size , fMaxSize);
            LOGW("Truncated");
            LOGW("Truncated");
            LOGW("Truncated");
            LOGW("Truncated");
            LOGW("Truncated");
            LOGW("Truncated");
            LOGW("Truncated");
            LOGW("Truncated");
            LOGW("Truncated");
            LOGW("Truncated");
            LOGW("Truncated");
            LOGW("Truncated");
        }
        else
        {
            fNumTruncatedBytes = 0;
        }

        // Cập nhật timestamp cho RTP Packet
        timeval tv;
        tv.tv_sec = static_cast<long>(Packet->pts);
        tv.tv_usec = static_cast<long>((Packet->pts - tv.tv_sec) * 1e6);
        fPresentationTime = tv;

        // Báo cho Live555 biết đã có Packet sẵn sàng
        FramedSource::afterGetting(this);
    }

    std::shared_ptr<Queue<UniquePacketPtr>> m_Queue;
};

#endif // CUSTOM_PACKET_SOURCE