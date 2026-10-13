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
    static H264LiveSubsession* createNew(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue, const AVRational& Timebase);

protected:
    H264LiveSubsession(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue, const AVRational& Timebase);
    FramedSource* createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) override;
    RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/) override;

private:
    std::shared_ptr<Queue<UniquePacketPtr>> m_Queue;
    AVRational m_Timebase;
};

#endif // H264_LIVE_SUB_SESSION