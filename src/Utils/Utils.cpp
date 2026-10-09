#include <vector>
#include <fstream>
#include <string>
#include "Utils.h"

Utils::Utils()
{

}

Utils& Utils::GetInstance()
{
    static Utils Instance;
    return Instance;
}

bool Utils::SavePPM(const std::string& filename, const std::vector<uint8_t>& rgb, int width, int height)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Head of PPM format Binary RGB (P6)
    file << "P6\n" << width << " " << height << "\n255\n";

    // Write file
    file.write(reinterpret_cast<const char*>(rgb.data()), rgb.size());
    file.close();
    return true;
}

double Utils::GetTimeStamp(const AVRational& Rational, uint64_t Time)
{
    if(Rational.num == 0)
    {
        return 0;
    }
    return ((double)Rational.num/Rational.den) * Time;
}

std::string Utils::GetSampleFormatName(const MediaType& MediaType, const int Format)
{
    std::string Retval;
    switch(MediaType)
    {
        case MediaType::AUDIO:
        {
            Retval = av_get_sample_fmt_name((AVSampleFormat)Format);
            break;
        }
        case MediaType::VIDEO:
        {
            Retval = av_get_pix_fmt_name((AVPixelFormat)Format);
            break;
        }
        default:
        {
            LOGE("Do not support this Mediatype");
        }
    }
    return Retval;
}

void Utils::ConvertTimestamp(const UniqueFramePtr& Frame , const AVRational& OldTimestamp, const AVRational& NewTimestamp)
{
    if((Frame == nullptr) || (av_cmp_q(OldTimestamp, NewTimestamp) == 0))
    {
        return;
    }
    if(Frame->pts == AV_NOPTS_VALUE)
    {
        LOGW("Timestamp invalid: AV_NOPTS_VALUE");
        return;
    }
    Frame->pts      = av_rescale_q(Frame->pts, OldTimestamp, NewTimestamp);
    Frame->pkt_duration = av_rescale_q(Frame->pkt_duration, OldTimestamp, NewTimestamp);
    Frame->pkt_dts = av_rescale_q(Frame->pkt_dts, OldTimestamp, NewTimestamp);
}

void Utils::ConvertTimestamp(const UniquePacketPtr& Packet , const AVRational& OldTimestamp, const AVRational& NewTimestamp)
{
    if((Packet == nullptr) || (av_cmp_q(OldTimestamp, NewTimestamp) == 0))
    {
        return;
    }
    if(Packet->pts == AV_NOPTS_VALUE)
    {
        LOGW("Timestamp invalid: AV_NOPTS_VALUE");
        return;
    }
    av_packet_rescale_ts(Packet.get(), OldTimestamp, NewTimestamp);
}

void Utils::PrintTimeStamp(const AVRational& Timebase, const uint64_t Timstamp)
{
    LOGI("Timebase: {}/{}", Timebase.num, Timebase.den);
    if(Timstamp != UINT64_MAX)
    {
        LOGI("Packet or Frame timestamp: {}", GetTimeStamp(Timebase, Timstamp));
    }
}