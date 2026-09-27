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

double Utils::GetTimeStamp(const AVRational& Rational, long long Time)
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