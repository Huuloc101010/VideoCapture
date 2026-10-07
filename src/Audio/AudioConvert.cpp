#include <algorithm>
#include "AudioConvert.h"

bool AudioConvert::Config(const AudioConvertConfig& Config)
{
    m_Config = Config;
    SwrContext* Swr = swr_alloc_set_opts(
        nullptr,
        // Output
        m_Config.ChannelLayout, // AV_CH_LAYOUT_STEREO,
        AV_SAMPLE_FMT_FLTP,     // Floating point Packed
        m_Config.SampleRate,    // 48000
        // Input
        m_Config.ChannelLayout, // AV_CH_LAYOUT_STEREO,
        AV_SAMPLE_FMT_S16,      // Siged 16bit per point
        m_Config.SampleRate,    // 48000
        0,
        nullptr
    );
    if(Swr == nullptr)
    {
        LOGE("Swr is nullptr");
        return false;
    }
    if(swr_init(Swr) < 0)
    {
        LOGE("Swr init fail");
        return false;
    }
    m_SwrContext.reset(Swr);
    LOGI("Config succeed");
    return true;
}

UniqueFramePtr AudioConvert::ConvertPacketToFrame(UniquePacketPtr Packet)
{
    if(Packet == nullptr)
    {
        LOGE("Packet is nullptr");
        return {};
    }
    UniqueFramePtr Frame(av_frame_alloc());
    AVCodecParameters* Par = m_Config.AudioStream->codecpar;
    AVSampleFormat Format = static_cast<AVSampleFormat>(Par->format);
    int BytesPerSample = av_get_bytes_per_sample(Format);
    int Channels = 2;
    int NbSamples = Packet->size / (BytesPerSample * Channels);
    Frame->format = AV_SAMPLE_FMT_S16;
    Frame->sample_rate = 48000;
    Frame->channel_layout = AV_CH_LAYOUT_STEREO;
    Frame->nb_samples = NbSamples;
    if(av_frame_get_buffer(Frame.get(), 0) != 0)
    {
        LOGE("Get buffer fail");
        return {};
    }
    std::copy(Packet->data, Packet->data + Packet->size, Frame->data[0]);
    return Frame;
}

UniqueFramePtr AudioConvert::ConvertS16ToFPTP(UniqueFramePtr Frame)
{
    if (m_SwrContext == nullptr || Frame == nullptr)
    {
        LOGE("Have something is nullptr");
        return nullptr;
    }

    UniqueFramePtr OutputFrame(av_frame_alloc());

    if (OutputFrame == nullptr)
    {
        return nullptr;
    }

    OutputFrame->format = AV_SAMPLE_FMT_FLTP;
    OutputFrame->sample_rate = m_Config.SampleRate;
    OutputFrame->channel_layout = m_Config.ChannelLayout;
    OutputFrame->channels = 2;
    OutputFrame->nb_samples = Frame->nb_samples;

    if (av_frame_get_buffer(OutputFrame.get(), 0) < 0)
    {
        LOGE("Get buffer fail");
        return nullptr;
    }

    int Ret = swr_convert(
        m_SwrContext.get(),

        // output
        OutputFrame->data,
        OutputFrame->nb_samples,

        // input
        const_cast<const uint8_t**>(Frame->data),
        Frame->nb_samples
    );

    if (Ret < 0)
    {
        LOGE("Convert fail");
        return nullptr;
    }

    OutputFrame->nb_samples = Ret;
    return OutputFrame;
}