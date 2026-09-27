#include "AlsaCapture.h"

bool AlsaCapture::Config(const AlsaCaptureConfig& Config)
{
    m_Config = Config;
    // Register with ffmpeg
    avdevice_register_all();

    const AVInputFormat* InputFormat = av_find_input_format("alsa");
    if (InputFormat == nullptr)
    {
        LOGE("alsa input format not found");
        return false;
    }

    AVDictionary* Opts = nullptr;
    av_dict_set(&Opts, "sample_rate", m_Config.SampleRate.c_str(), 0);
    av_dict_set(&Opts, "channels", m_Config.Channels.c_str(), 0);
    AVFormatContext* FormatContext = nullptr;
    int Retval = avformat_open_input(&FormatContext, m_Config.Device.c_str(), InputFormat, &Opts);
    m_FormatContext.reset(FormatContext);
    if(m_FormatContext == nullptr)
    {
        LOGE("Format context is null");
        return false;
    }
    av_dict_free(&Opts);

    if(Retval < 0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(Retval, errbuf, sizeof(errbuf));
        LOGE("Can not open device {}: {}", m_Config.Device, errbuf);
        return false;
    }

    if(m_FormatContext == nullptr)
    {
        LOGE("m_FormatContext == nullptr");
        return false;
    }

    if (avformat_find_stream_info(m_FormatContext.get(), nullptr) < 0)
    {
        LOGE("Can not find stream info {}", m_Config.Device);
        return false;
    }

    m_AudioStreamIndex = -1;
    for (int i = 0; i < m_FormatContext->nb_streams; ++i)
    {
        if (m_FormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            m_AudioStreamIndex = i;
            break;
        }
    }

    if (m_AudioStreamIndex < 0)
    {
        LOGE("Can not find audio stream in {}", m_Config.Device);
        return false;
    }
    AVStreamPtr Stream = GetStream();

    LOGI("sample_rate = {}", Stream->codecpar->sample_rate);
    //LOGI("channels    = {}", Stream->codecpar->ch_layout.nb_channels);
    LOGI("format      = {}", Stream->codecpar->format);
    LOGI("codec_id    = {}", (int)Stream->codecpar->codec_id);

    LOGI("Alsa init success");
    return true;
}

UniquePacketPtr AlsaCapture::ReadPacket()
{
    for (int i = 0; i < 5; ++i)
    {
        UniquePacketPtr Packet(av_packet_alloc());

        int Retval = av_read_frame(
            m_FormatContext.get(),
            Packet.get()
        );

        if(Retval != 0)
        {
            if (Retval == AVERROR_EOF)
            {
                LOGE("EOF Alsa stream");
                return {};
            }

            if (Retval == AVERROR(EAGAIN))
            {
                continue;
            }

            char ErrBuffer[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(Retval, ErrBuffer, sizeof(ErrBuffer));

            LOGW("Error read alsa: {}, Trying {} time left", ErrBuffer, 5 - i);

            continue;
        }
        // Retval == 0 => read successfully
        if (Packet->stream_index == m_AudioStreamIndex)
        {
            return Packet;
        }
    }

    return {};
}

AVStreamPtr AlsaCapture::GetStream()
{
    return (m_FormatContext && m_AudioStreamIndex >= 0) ? m_FormatContext->streams[m_AudioStreamIndex] : nullptr;
}