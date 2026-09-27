#include <algorithm>
#include "AudioConvert.h"

UniqueFramePtr AudioConvert::ConvertPacketToFrame(UniquePacketPtr Packet)
{
    if(Packet == nullptr)
    {
        LOGE("Packet is nullptr");
        return {};
    }
    UniqueFramePtr Frame(av_packet_alloc());
    AVCodecParameters* Par = Stream->codecpar;
    AVSampleFormat Format = static_cast<AVSampleFormat>(Par->format);
    int BytesPerSample = av_get_bytes_per_sample(Format);
    int Channels = Par->ch_layout.nb_channels;
    int NbSamples = Packet->size / (BytesPerSample * Channels);
    Frame->format = S16;
    Frame->sample_rate = ;
    Frame->ch_layout = ;
    Frame->nb_samples = ;
    if(av_frame_get_buffer(Frame.get(), 0) != 0)
    {
        LOGE("Get buffer fail");
        return {};
    }
    std::copy(Packet->data, Packet->data + Packet->size, Frame->data[0]);
    return Frame;
}