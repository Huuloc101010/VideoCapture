#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>
#include <stdint.h>
#include <cstdint>
#include <limits>
#include "Define.h"
#include "Log.h"

class Utils
{
public:
    Utils();
    static Utils& GetInstance();
    bool SavePPM(const std::string& filename, const std::vector<uint8_t>& rgb, int width, int height);
    double GetTimeStamp(const AVRational& Rational, int64_t Time);
    int64_t GetTimeStamp(const AVRational& Rational, double Time);
    std::string GetSampleFormatName(const MediaType& MediaType, const int Format);
    void ConvertTimestamp(const UniqueFramePtr& Frame , const AVRational& OldTimestamp, const AVRational& NewTimestamp);
    void ConvertTimestamp(const UniquePacketPtr& Packet , const AVRational& OldTimestamp, const AVRational& NewTimestamp);
    void PrintTimeStamp(const AVRational& Timebase, const int64_t Timstamp = UINT64_MAX);
private:
    
};

#endif // UTILS_H
