#include "Muxer.h"
#include "Utils.h"

bool Muxer::Config(const MuxerConfig& Config)
{
    m_Config = Config;
    AVFormatContext* FormatContext = nullptr;
    int Ret = avformat_alloc_output_context2(&FormatContext, nullptr, Config.Extension.c_str(), Config.VideoName.c_str());
    if(Ret < 0 || FormatContext == nullptr)
    {
        LOGE("Alloc context fail");
        return false;
    }
    m_FormatContext.reset(FormatContext);

    // Create new stream
    m_VideoStream = avformat_new_stream(m_FormatContext.get(), nullptr);
    if(m_VideoStream == nullptr)
    {
        LOGE("Video stream == nullptr");
        return false;
    }
    LOGI(
    "Muxer stream TB = {}/{}",
    m_VideoStream->time_base.num,
    m_VideoStream->time_base.den
    );
    if (Config.VideoCodecContex == nullptr)
    {
        LOGE("VideoCodecContex is nullptr");
        return false;
    }
    
    // Copy encoder information -> stream codecpar
    Ret = avcodec_parameters_from_context(m_VideoStream->codecpar, Config.VideoCodecContex);
    if (Ret < 0)
    {
        LOGE("avcodec_parameters_from_context failed");
        return false;
    }

    // Use encoder time base
    m_VideoStream->time_base = Config.VideoCodecContex->time_base;

    // Open output file
    if(!(FormatContext->oformat->flags & AVFMT_NOFILE))
    {
        Ret = avio_open(&m_FormatContext->pb, Config.VideoName.c_str() , AVIO_FLAG_WRITE);

        if (Ret < 0)
        {
            LOGE("Failed to open output file");
            return false;
        }
    }

    LOGI(
        "Muxer config success: {}x{}, time_base={}/{}",
        m_VideoStream->codecpar->width,
        m_VideoStream->codecpar->height,
        m_VideoStream->time_base.num,
        m_VideoStream->time_base.den
    );
    LOGI("Config Demuxer success");
    return true;
}

bool Muxer::WriteHeader()
{
    int Ret = avformat_write_header(m_FormatContext.get(), nullptr);

    if (Ret < 0)
    {
        LOGE("Write header failed");
        return false;
    }

    return true;
}

bool Muxer::WriteTrailer()
{
    int Ret = av_write_trailer(m_FormatContext.get());

    if (m_FormatContext->pb)
    {
        avio_closep(&m_FormatContext->pb);
    }

    return Ret >= 0;
}

bool Muxer::WritePacket(const UniquePacketPtr Packet)
{
    if(Packet == nullptr)
    {
        LOGE("Packet is nullptr");
        return false;
    }
    if(m_VideoStream == nullptr)
    {
        LOGE("Video Stream is null");
        return false;
    }
    Packet->stream_index = m_VideoStream->index;
    Utils::GetInstance().ConvertTimestamp(Packet, m_Config.VideoCodecContex->time_base, m_VideoStream->time_base);
    int Ret = av_interleaved_write_frame(m_FormatContext.get(), Packet.get());
    if(Ret < 0)
    {
        LOGE("Write frame fail");
        return false;
    }
    return true;
}

AVRational Muxer::GetTimeBase()
{
    if(m_VideoStream == nullptr)
    {
        LOGE("m_VideoStream is null");
        return {};
    }
    return m_VideoStream->time_base;
}