#include "AudioEncoder.h"

bool AudioEncoder::ConfigEncoder(const EncoderConfig& Config)
{
    m_Codec = (AVCodecPtr)avcodec_find_encoder(AV_CODEC_ID_AAC);
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
    m_CodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
    m_CodecContext->sample_rate = 48000;
    m_CodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
    m_CodecContext->bit_rate = 128000;
    /* frames per second */
    // m_CodecContext->time_base = (AVRational){1, 1000000};
    // m_CodecContext->framerate = (AVRational){30, 1};
    // m_CodecContext->gop_size = 10;
    // m_CodecContext->max_b_frames = 0;
    // m_CodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    // if(m_Codec->id == AV_CODEC_ID_AAC)
    // {
    //     av_opt_set(m_CodecContext->priv_data, "preset", "slow", 0);
    //     av_opt_set(m_CodecContext->priv_data, "tune", "zerolatency", 0);
    // }
    if(avcodec_open2(m_CodecContext.get(), m_Codec, NULL) != 0)
    {
        LOGE("avcodec_open2() return false");
        return false;
    }
    LOGI("avcodec_open2() success");
    return true;
}