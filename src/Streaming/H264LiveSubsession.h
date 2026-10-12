#ifndef H264_LIVE_SUB_SESSION
#define H264_LIVE_SUB_SESSION

#include <OnDemandServerMediaSubsession.hh>
#include <UsageEnvironment.hh>
#include <CustomPacketSource.h>
#include "Queue.h"
#include "Define.h"

class H264LiveSubsession : public OnDemandServerMediaSubsession
{
public:
    static H264LiveSubsession* createNew(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue)
    {
        return new H264LiveSubsession(env, queue);
    }

protected:
    H264LiveSubsession(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue)
        : OnDemandServerMediaSubsession(env, True /* reuseFirstSource */), m_Queue(queue) {}

    FramedSource* createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) override
    {
        estBitrate = 2000; // kbps
        auto source = CustomPacketSource::createNew(envir(), m_Queue);
        // Use H264VideoStreamFramer for auto parse Annex-B NALUs (0x00000001)
        return H264VideoStreamFramer::createNew(envir(), source);
    }

    RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/) override
    {
        return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
    }

private:
    std::shared_ptr<Queue<UniquePacketPtr>> m_Queue;
};

#endif // H264_LIVE_SUB_SESSION