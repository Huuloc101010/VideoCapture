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
    Frame->pts = Packet->pts;
    Frame->pkt_dts = Packet->pts;
    Frame->pkt_duration = Packet->duration;
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
    OutputFrame->pkt_dts = Frame->pkt_dts;
    OutputFrame->pts = Frame->pts;
    OutputFrame->pkt_duration = Frame->pkt_duration;
    return OutputFrame;
}

// Spit Frame 4096 to 1024
std::vector<UniqueFramePtr> AudioConvert::SplitPacket(UniqueFramePtr Frame)
{
    if (Frame == nullptr || Frame->nb_samples != 4096)
    {
        LOGE("Frame is null or nb_samples != 4096");
        return {};
    }

    constexpr int TARGET_SAMPLES = 1024;
    constexpr int NUM_SPLITS = 4;
    std::vector<UniqueFramePtr> Retval(NUM_SPLITS);

    #if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100)
        int num_channels = Frame->ch_layout.nb_channels;
    #else
        int num_channels = av_get_channel_layout_nb_channels(Frame->channel_layout);
    #endif

    int sample_size = av_get_bytes_per_sample(static_cast<AVSampleFormat>(Frame->format));
    bool is_planar = av_sample_fmt_is_planar(static_cast<AVSampleFormat>(Frame->format));

    int64_t pts_offset = 0;

    for (int i = 0; i < NUM_SPLITS; ++i)
    {
        Retval[i].reset(av_frame_alloc());
        Retval[i]->nb_samples     = TARGET_SAMPLES;
        Retval[i]->format         = Frame->format;
        Retval[i]->sample_rate    = Frame->sample_rate;

        #if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100)
            av_channel_layout_copy(&Retval[i]->ch_layout, &Frame->ch_layout);
        #else
            Retval[i]->channel_layout = Frame->channel_layout;
        #endif

        if (av_frame_get_buffer(Retval[i].get(), 0) < 0)
        {
            LOGE("Get buffer failed for frame {}", i);
            return {};
        }

        // Copy audio
        int sample_offset = i * TARGET_SAMPLES;

        if (is_planar)
        {
            // Copy each channel (data[0], data[1], ...)
            for (int ch = 0; ch < num_channels; ++ch)
            {
                uint8_t* src_ptr = Frame->data[ch] + (sample_offset * sample_size);
                uint8_t* dst_ptr = Retval[i]->data[ch];
                memcpy(dst_ptr, src_ptr, TARGET_SAMPLES * sample_size);
            }
        }
        else
        {
            // Packed format (Interleaved L-R-L-R): All channel in data[0]
            uint8_t* src_ptr = Frame->data[0] + (sample_offset * num_channels * sample_size);
            uint8_t* dst_ptr = Retval[i]->data[0];
            memcpy(dst_ptr, src_ptr, TARGET_SAMPLES * num_channels * sample_size);
        }

        // Update PTS & Duration
        Retval[i]->pts = Frame->pts + (i * TARGET_SAMPLES);
        Retval[i]->pkt_duration = TARGET_SAMPLES;
    }

    return Retval;
}