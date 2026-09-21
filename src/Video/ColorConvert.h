#ifndef COLOR_CONVERT_H
#define COLOR_CONVERT_H

#include <vector>
#include <cstdint>
#include "Define.h"
#include "Log.h"

class ColorConvert
{
public:
    ColorConvert(ColorConvertConfig Config);
    ~ColorConvert();
    std::vector<uint8_t> YUV422ToRGB24(const std::vector<uint8_t>& yuyv, int width, int height);
    std::vector<uint8_t> YUV420ToRGB24(const std::vector<uint8_t>& yuv420, int width, int height);
    UniqueFramePtr ConvertPacketToFrame(UniquePacketPtr Packet);
    UniqueFramePtr ConvertYUV422ToYUV420(UniqueFramePtr Frame); // Support only YUYV
private:
    ColorConvertConfig m_Config;
    SwsContext* m_SwsContext;
};

#endif // COLOR_CONVERT_H