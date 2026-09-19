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
    V4l2CaptureConfig Config = {type2, "/dev/video0", width, height, 30, AV_PIX_FMT_YUYV422};
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
    UniqueFramePtr frame = Capture.ConvertPacketToFrame(std::move(packet));
    if(frame == nullptr)
    {
        LOGE("frame is nullptr");
        return -1;
    }
    int size = frame->width * frame->height * 2;
    LOGI("Size: {}", size);
    std::vector<uint8_t> rgb((frame->data[0]), frame->data[0] + size);
    for(int i = 0; i < size; ++i)
    {
        rgb[i] = frame->data[0][i];
    }
    ColorConvert Convert;
    std::vector<uint8_t> Ret = Convert.YUV422ToRGB24(rgb,  width, height);
    Utils::GetInstance().SavePPM("tmp/abc.ppm", Ret, width, height);
    return 0;
}
