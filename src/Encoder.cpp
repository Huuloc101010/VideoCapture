#include "Encoder.h"


std::vector<UniquePacketPtr> Encoder::Encode(UniqueFramePtr Frame)
{
    std::vector<UniquePacketPtr> VectorPacket;
    int Retval = avcodec_send_frame(m_CodecContext.get(), Frame.get());
    if(Retval != 0)
    {
        LOGE("avcodec_send_frame() fail");
        return VectorPacket;
    }

    int Ret = 0;
    while(Ret >= 0)
    {
        UniquePacketPtr Packet(av_packet_alloc());
        Ret = avcodec_receive_packet(m_CodecContext.get(), Packet.get());
        if(Ret == AVERROR(EAGAIN) || Ret == AVERROR_EOF)
        {
            //LOGE("Ret == AVERROR(EAGAIN)");
            break;
        }
        else if(Ret < 0)
        {
            LOGE("Error when encoding");
            break;
        }
        else
        {
            VectorPacket.emplace_back(std::move(Packet));
        }

    }
    //LOGW("OK, Get {} Packet", (int)VectorPacket.size());
    return VectorPacket;
}

const UniqueCodecContext& Encoder::GetCodecContext() const
{
    return m_CodecContext;
}

AVRational Encoder::GetTimeBase()
{
    if(m_CodecContext == nullptr)
    {
        LOGE("Codecontext is null");
        return {};
    }
    return m_CodecContext->time_base;
}