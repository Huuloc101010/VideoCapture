#ifndef AAC_LIVE_SUB_SESSION
#define AAC_LIVE_SUB_SESSION

#include <memory>
#include <OnDemandServerMediaSubsession.hh>
#include <MPEG4GenericRTPSink.hh>
#include "CustomPacketSource.h"
#include "Queue.h"
#include "Define.h"

class AACLiveSubsession : public OnDemandServerMediaSubsession
{
public:
    static AACLiveSubsession* createNew(UsageEnvironment& Env, std::shared_ptr<Queue<UniquePacketPtr>> Queue, 
                                        int SampleRate = 48000, int Channels = 2, const AVRational& Timebase = {});
protected:
    AACLiveSubsession(UsageEnvironment& Env, std::shared_ptr<Queue<UniquePacketPtr>> Queue, int SampleRate, int Channels, const AVRational& Timebase);
    FramedSource* createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) override;
    RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/) override;

private:
    std::shared_ptr<Queue<UniquePacketPtr>> m_Queue;
    int m_sampleRate;
    int m_channels;
    AVRational m_Timebase;
};

#endif // AAC_LIVE_SUB_SESSION