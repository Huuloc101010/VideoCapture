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
                                        int SampleRate = 48000, int Channels = 2)
    {
        return new AACLiveSubsession(Env, Queue, SampleRate, Channels);
    }

protected:
    AACLiveSubsession(UsageEnvironment& Env, std::shared_ptr<Queue<UniquePacketPtr>> Queue, int SampleRate, int Channels)
        : OnDemandServerMediaSubsession(Env, true), m_Queue(Queue), m_sampleRate(SampleRate), m_channels(Channels)
        {

        }

    FramedSource* createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) override
    {
        estBitrate = 128; // kbps
        return CustomPacketSource::createNew(envir(), m_Queue);
    }

    RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/) override {
        // MPEG4GenericRTPSink support AAC (MPEG-4 Audio) via RTP
        return MPEG4GenericRTPSink::createNew(
            envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
            m_sampleRate, "audio", "AAC-hbr", "", m_channels
        );
    }

private:
    std::shared_ptr<Queue<UniquePacketPtr>> m_Queue;
    int m_sampleRate;
    int m_channels;
};

#endif // AAC_LIVE_SUB_SESSION