#include "Muxer.h"
#include "Utils.h"

bool Muxer::Config(const MuxerConfig& Config)
{
    // Create format context for output video
    // Format context represent for output video
    m_Config = Config;
    AVFormatContext* FormatContext = nullptr;
    int Ret = avformat_alloc_output_context2(&FormatContext, nullptr, Config.Extension.c_str(), Config.VideoName.c_str());
    if(Ret < 0 || FormatContext == nullptr)
    {
        LOGE("Alloc context fail");
        return false;
    }
    m_FormatContext.reset(FormatContext);
    // Create video stream
    if(ConfigVideo() == false)
    {
        LOGE("Config video muxer fail");
        return false;
    }
    // Create audio stream
    if(ConfigAudio() == false)
    {
        LOGE("Config audio muxer fail");
        return false;
    }
    // Open output file
    if(!(m_FormatContext->oformat->flags & AVFMT_NOFILE))
    {
        Ret = avio_open(&m_FormatContext->pb, m_Config.VideoName.c_str() , AVIO_FLAG_WRITE);

        if(Ret < 0)
        {
            LOGE("Failed to open output file");
            return false;
        }
    }

    LOGI("Config Demuxer success");
    return true;
}

bool Muxer::ConfigVideo()
{
    // Create new stream
    m_VideoStream = avformat_new_stream(m_FormatContext.get(), nullptr);
    if(m_VideoStream == nullptr)
    {
        LOGE("Video stream == nullptr");
        return false;
    }
    if (m_Config.VideoCodecContext == nullptr)
    {
        LOGE("VideoCodecContext is nullptr");
        return false;
    }
    
    // Copy encoder information -> stream codecpar
    int Ret = avcodec_parameters_from_context(m_VideoStream->codecpar, m_Config.VideoCodecContext);
    if (Ret < 0)
    {
        LOGE("avcodec_parameters_from_context failed");
        return false;
    }

    // Use encoder time base
    m_VideoStream->time_base = m_Config.VideoCodecContext->time_base;

    return true;
}

bool Muxer::ConfigAudio()
{
    // Create audio stream
    m_AudioStream = avformat_new_stream(m_FormatContext.get(), nullptr);
    if(m_AudioStream == nullptr)
    {
        LOGE("Audio stream == nullptr");
        return false;
    }
    // if (m_Config.AudioCodecContext == nullptr)
    // {
    //     LOGE("AudioCodecContext is nullptr");
    //     return false;
    // }
    
    // Copy encoder information -> stream codecpar
    int Ret = avcodec_parameters_from_context(m_AudioStream->codecpar, m_Config.VideoCodecContext);
    if (Ret < 0)
    {
        LOGE("avcodec_parameters_from_context failed");
        return false;
    }

    // Use encoder time base
    m_AudioStream->time_base = m_Config.AudioCodecContext->time_base;

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

    if(m_FormatContext->pb)
    {
        avio_closep(&m_FormatContext->pb);
        m_FormatContext->pb = nullptr;
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