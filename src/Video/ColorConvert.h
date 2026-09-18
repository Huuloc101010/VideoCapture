#ifndef COLOR_CONVERT_H
#define COLOR_CONVERT_H

#include <vector>
#include <cstdint>

class ColorConvert
{
public:
    ColorConvert() = default;
    std::vector<uint8_t> YUV422ToRGB24(const std::vector<uint8_t>& yuyv, int width, int height);
private:
};

#endif // COLOR_CONVERT_H