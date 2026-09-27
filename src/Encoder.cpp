#include "Encoder.h"

bool Encoder::ConfigEncoder(const EncoderConfig& Config)
{
    m_Codec = (AVCodecPtr)avcodec_find_encoder(AV_CODEC_ID_H264);
    if(m_Codec == nullptr)
    {
        LOGE("Can not find decoder");
        return false;
    }
    m_CodecContext.reset(avcodec_alloc_context3(m_Codec));
    if(m_CodecContext == nullptr)
    {
        LOGE("m_CodecContext = nullptr");
        return false;
    }
    m_CodecContext->bit_rate = 400000;
    m_CodecContext->width = Config.Width;
    m_CodecContext->height = Config.Height;
    /* frames per second */
    m_CodecContext->time_base = (AVRational){1, 1000000};
    m_CodecContext->framerate = (AVRational){30, 1};
    m_CodecContext->gop_size = 10;
    m_CodecContext->max_b_frames = 0;
    m_CodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    if(m_Codec->id == AV_CODEC_ID_H264)
    {
        av_opt_set(m_CodecContext->priv_data, "preset", "slow", 0);
        av_opt_set(m_CodecContext->priv_data, "tune", "zerolatency", 0);
    }
    if(avcodec_open2(m_CodecContext.get(), m_Codec, NULL) != 0)
    {
        LOGE("avcodec_open2() return false");
        return false;
    }
    LOGI("avcodec_open2() success");
    return true;
}

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

const UniqueCodecContext& Encoder::GetVideoContext() const
{
    return m_CodecContext;
}

