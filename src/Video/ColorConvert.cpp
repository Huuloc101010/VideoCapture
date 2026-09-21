#include <algorithm>
#include "ColorConvert.h"

ColorConvert::ColorConvert(ColorConvertConfig Config)
{
    m_Config = Config;
}

std::vector<uint8_t> ColorConvert::YUV422ToRGB24(const std::vector<uint8_t>&  yuyv, int width, int height)
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

std::vector<uint8_t> ColorConvert::YUV420ToRGB24(const std::vector<uint8_t>& yuv420, int width, int height)
{
    int frameSize = width * height;
    int chromaSize = frameSize / 4;
    
    // Check if the input buffer has enough data for YUV420 (width * height * 1.5)
    if (yuv420.size() < static_cast<size_t>(frameSize + 2 * chromaSize))
    {
        return {}; 
    }

    std::vector<uint8_t> rgb24(frameSize * 3);

    // Locate the starting points for Y, U, and V planes (I420 Planar format)
    const uint8_t* yPlane = yuv420.data();
    const uint8_t* uPlane = yuv420.data() + frameSize;
    const uint8_t* vPlane = yuv420.data() + frameSize + chromaSize;

    int rgbIdx = 0;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            // Get the Y value for the current pixel
            int Y = yPlane[y * width + x];

            // In YUV420, every 2x2 pixel block shares one U and one V sample
            int uvIdx = (y / 2) * (width / 2) + (x / 2);
            int U = uPlane[uvIdx] - 128;
            int V = vPlane[uvIdx] - 128;

            // BT.601 conversion formula from YUV to RGB
            int R = Y + static_cast<int>(1.402f * V);
            int G = Y - static_cast<int>(0.344136f * U + 0.714136f * V);
            int B = Y + static_cast<int>(1.772f * U);

            // Clamp the values to the valid 0-255 range to prevent overflow
            rgb24[rgbIdx++] = static_cast<uint8_t>(std::clamp(R, 0, 255)); // Red
            rgb24[rgbIdx++] = static_cast<uint8_t>(std::clamp(G, 0, 255)); // Green
            rgb24[rgbIdx++] = static_cast<uint8_t>(std::clamp(B, 0, 255)); // Blue
        }
    }

    return rgb24;
}

UniqueFramePtr ColorConvert::ConvertPacketToFrame(UniquePacketPtr Packet)
{
    if (Packet == nullptr)
    {
        LOGE("Packet is nullptr");
        return {};
    }

    UniqueFramePtr Frame(av_frame_alloc());

    if (Frame == nullptr)
    {
        LOGE("Allocate fail");
        return {};
    }

    Frame->format = m_Config.PixelFormat;
    Frame->width  = m_Config.Width;
    Frame->height = m_Config.Heigh;
    LOGI("Format {}", (int)m_Config.PixelFormat);
    LOGI("Width {}", m_Config.Width);
    LOGI("Heigh {}", m_Config.Heigh);
    int Ret = av_frame_get_buffer(Frame.get(), 32);

    if (Ret < 0)
    {
        LOGE("av_frame_get_buffer() failed");
        return {};
    }

    // YUYV422 = 2 bytes / pixel
    const int SrcLinesize[4] =
    {
        m_Config.Width * 2,
        0,
        0,
        0
    };

    const uint8_t* SrcData[4] =
    {
        Packet->data,
        nullptr,
        nullptr,
        nullptr
    };

    av_image_copy(
        Frame->data,
        Frame->linesize,
        SrcData,
        SrcLinesize,
        m_Config.PixelFormat,
        m_Config.Width,
        m_Config.Heigh
    );

    Frame->pts = Packet->pts;
    return Frame;
}

UniqueFramePtr ColorConvert::ConvertYUV422ToYUV420(UniqueFramePtr Frame)
{
    if(Frame == nullptr)
    {
        LOGE("Frame is nullptr");
        return nullptr;
    }

    if(Frame->format != AV_PIX_FMT_YUYV422)
    {
        LOGE("Invalid source format: {}", Frame->format);
        return nullptr;
    }

    UniqueFramePtr YUV420(av_frame_alloc());
    if (YUV420 == nullptr)
    {
        LOGE("av_frame_alloc failed");
        return nullptr;
    }

    YUV420->format = AV_PIX_FMT_YUV420P;
    YUV420->width  = Frame->width;
    YUV420->height = Frame->height;

    // FFmpeg allocates Y/U/V planes
    if(av_frame_get_buffer(YUV420.get(), 32) < 0)
    {
        LOGE("av_frame_get_buffer failed");
        return nullptr;
    }

    // ---------------------------------------------------------
    // YUYV422 -> YUV420P
    // ---------------------------------------------------------

    SwsContext* sws = sws_getContext(
        Frame->width,
        Frame->height,
        AV_PIX_FMT_YUYV422,

        YUV420->width,
        YUV420->height,
        AV_PIX_FMT_YUV420P,
        SWS_BILINEAR,
        nullptr,
        nullptr,
        nullptr
    );

    if(sws == nullptr)
    {
        LOGE("sws_getContext failed");
        return nullptr;
    }

    const int scaledHeight = sws_scale(
        sws,
        Frame->data,
        Frame->linesize,

        0,
        YUV420->height,

        YUV420->data,
        YUV420->linesize
    );

    if (scaledHeight != YUV420->height)
    {
        LOGE("sws_scale failed: {}/{}", scaledHeight, YUV420->height);

        sws_freeContext(sws);
        return nullptr;
    }

    YUV420->pts = Frame->pts;

    sws_freeContext(sws);

    return YUV420;
}