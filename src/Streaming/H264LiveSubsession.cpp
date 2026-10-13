#include "H264LiveSubsession.h"

H264LiveSubsession* H264LiveSubsession::createNew(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue, const AVRational& Timebase)
{
    return new H264LiveSubsession(env, queue, Timebase);
}

H264LiveSubsession::H264LiveSubsession(UsageEnvironment& env, std::shared_ptr<Queue<UniquePacketPtr>> queue, const AVRational& Timebase)
    : OnDemandServerMediaSubsession(env, True /* reuseFirstSource */), m_Queue(queue)
{
    m_Timebase = Timebase;
}

FramedSource* H264LiveSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
{
    estBitrate = 2000; // kbps
    auto source = CustomPacketSource::createNew(envir(), m_Queue, m_Timebase);
    // Use H264VideoStreamFramer for auto parse Annex-B NALUs (0x00000001)
    return H264VideoStreamFramer::createNew(envir(), source);
}

RTPSink* H264LiveSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/)
{
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
