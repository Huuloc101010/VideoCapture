#include <algorithm>
#include "ColorConvert.h"

std::vector<uint8_t> ColorConvert::YUV422ToRGB24(std::vector<uint8_t>  yuyv, int width, int height)
{
    std::vector<uint8_t> rgb(width * height * 3);
    
    auto clamp = [](int val) -> uint8_t {
        return static_cast<uint8_t>(std::clamp(val, 0, 255));
    };

    size_t yuyv_idx = 0;
    size_t rgb_idx = 0;

    for (int i = 0; i < (width * height) / 2; ++i) {
        int y0 = yuyv[yuyv_idx++];
        int u  = yuyv[yuyv_idx++] - 128;
        int y1 = yuyv[yuyv_idx++];
        int v  = yuyv[yuyv_idx++] - 128;

        // cal YUV BT.601 -> RGB
        int c0 = y0 - 16;
        int c1 = y1 - 16;

        // Pixel 1
        rgb[rgb_idx++] = clamp((298 * c0 + 409 * v + 128) >> 8);           // R
        rgb[rgb_idx++] = clamp((298 * c0 - 100 * u - 208 * v + 128) >> 8);   // G
        rgb[rgb_idx++] = clamp((298 * c0 + 516 * u + 128) >> 8);           // B

        // Pixel 2
        rgb[rgb_idx++] = clamp((298 * c1 + 409 * v + 128) >> 8);           // R
        rgb[rgb_idx++] = clamp((298 * c1 - 100 * u - 208 * v + 128) >> 8);   // G
        rgb[rgb_idx++] = clamp((298 * c1 + 516 * u + 128) >> 8);           // B
    }

    return rgb;
}