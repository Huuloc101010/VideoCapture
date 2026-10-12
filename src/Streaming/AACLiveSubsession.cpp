#include "AACLiveSubsession.h"

AACLiveSubsession::AACLiveSubsession(UsageEnvironment& Env, std::shared_ptr<Queue<UniquePacketPtr>> Queue, int SampleRate, int Channels)
    : OnDemandServerMediaSubsession(Env, true), m_Queue(Queue), m_sampleRate(SampleRate), m_channels(Channels)
{

}

AACLiveSubsession* AACLiveSubsession::createNew(UsageEnvironment& Env, std::shared_ptr<Queue<UniquePacketPtr>> Queue, 
                                    int SampleRate, int Channels)
{
    return new AACLiveSubsession(Env, Queue, SampleRate, Channels);
}

FramedSource* AACLiveSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
{
    estBitrate = 128; // kbps
    return CustomPacketSource::createNew(envir(), m_Queue);
}

RTPSink* AACLiveSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/)
{
    // MPEG4GenericRTPSink support AAC (MPEG-4 Audio) via RTP
    return MPEG4GenericRTPSink::createNew(
        envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
        m_sampleRate, "audio", "AAC-hbr", "", m_channels
    );
}