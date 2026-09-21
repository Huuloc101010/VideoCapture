#include "VideoRecorder.h"
#include "V4l2Capture.h"
#include "Utils.h"
#include "ColorConvert.h"
#include "Define.h"

int main()
{
    constexpr int width = 640;
    constexpr int height = 480;
    CaptureType type = CaptureType::V4L2_NATIVE;
    CaptureType type2 = CaptureType::FFMPEG_CAPTURE;
    V4l2Capture Capture;
    V4l2CaptureConfig Config = {type2, "/dev/video2", width, height, 30, AV_PIX_FMT_YUYV422};
    LOGW("type {}", (int)type);
    if(Capture.Config(Config) == false)
    {
        LOGE("Config fail");
        return -1;
    }
    LOGI("Config success");
    UniquePacketPtr packet = Capture.ReadPacket();
    if(packet == nullptr)
    {
        LOGE("packet is null");
        return -1;
    }
    ColorConvertConfig ConvertConfig = {width, height, AV_PIX_FMT_YUYV422};
    ColorConvert Convert(ConvertConfig);
    UniqueFramePtr frame = Convert.ConvertPacketToFrame(std::move(packet));
    if(frame == nullptr)
    {
        LOGE("frame is nullptr");
        return -1;
    }
    UniqueFramePtr YUV420 = Convert.ConvertYUV422ToYUV420(std::move(frame));
    if(YUV420 == nullptr)
    {
        LOGE("YUV420 is nullptr");
        return -1;
    }
    
    int size_yuv420 = YUV420->width * YUV420->height * 1.5; 
    LOGI("YUV420 Size: {}", size_yuv420); 

    // 1. Allocate a flat memory buffer for continuous YUV420P data
    std::vector<uint8_t> yuv420_data(size_yuv420);
    uint8_t* dst = yuv420_data.data();

    // 2. Extract Y Plane (Plane 0) - Size: width * height
    // Copy row by row to discard padding memory specified by linesize[0]
    for (int i = 0; i < YUV420->height; ++i)
    {
        std::copy(YUV420->data[0] + i * YUV420->linesize[0],
                  YUV420->data[0] + i * YUV420->linesize[0] + YUV420->width,
                  dst + i * YUV420->width);
    }

    // Advance destination pointer past Y data block
    dst += YUV420->width * YUV420->height;

    // 3. Extract U Plane (Plane 1) - Size: (width / 2) * (height / 2)
    // Copy row by row to discard padding memory specified by linesize[1]
    for (int i = 0; i < YUV420->height / 2; ++i)
    {
        std::copy(YUV420->data[1] + i * YUV420->linesize[1],
                  YUV420->data[1] + i * YUV420->linesize[1] + (YUV420->width / 2),
                  dst + i * (YUV420->width / 2));
    }

    // Advance destination pointer past U data block
    dst += (YUV420->width / 2) * (YUV420->height / 2);

    // 4. Extract V Plane (Plane 2) - Size: (width / 2) * (height / 2)
    // Copy row by row to discard padding memory specified by linesize[2]
    for (int i = 0; i < YUV420->height / 2; ++i)
    {
        std::copy(YUV420->data[2] + i * YUV420->linesize[2],
                  YUV420->data[2] + i * YUV420->linesize[2] + (YUV420->width / 2),
                  dst + i * (YUV420->width / 2));
    }

    // 5. Convert the freshly packed flat YUV420P buffer into RGB24
    std::vector<uint8_t> Ret = Convert.YUV420ToRGB24(yuv420_data, width, height); 

    Utils::GetInstance().SavePPM("tmp/abc_420.ppm", Ret, width, height); 

    return 0; 
}
