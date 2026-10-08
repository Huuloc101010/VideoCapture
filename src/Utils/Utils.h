#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>
#include <stdint.h>
#include "Define.h"
#include "Log.h"

class Utils
{
public:
    Utils();
    static Utils& GetInstance();
    bool SavePPM(const std::string& filename, const std::vector<uint8_t>& rgb, int width, int height);
    double GetTimeStamp(const AVRational& Rational, long long Time);
    std::string GetSampleFormatName(const MediaType& MediaType, const int Format);
    void ConvertTimestamp(const UniqueFramePtr& Frame , const AVRational& OldTimestamp, const AVRational& NewTimestamp);
    void ConvertTimestamp(const UniquePacketPtr& Packet , const AVRational& OldTimestamp, const AVRational& NewTimestamp);
private:
    
};

#endif // UTILS_H
