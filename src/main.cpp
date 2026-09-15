#include "VideoRecorder.h"
#include "V4l2Capture.h"
#include "Utils.h"
#include "ColorConvert.h"
#include "Define.h"


int main()
{
	
    // VideoRecorder Record;
    // Record.Capture();

    constexpr int width = 640;
    constexpr int height = 480;
    V4l2Capture Capture;
    V4l2CaptureConfig Config = {"/dev/video2", width, height, 30};
    Capture.Config(Config);
    UniquePacketPtr packet = Capture.ReadPacket();
    UniqueFramePtr frame = Capture.ConvertPacketToFrame(std::move(packet));
    int size = frame->width * frame->height * 2;
    std::cout << size << std::endl;
    std::vector<uint8_t> rgb(*frame->data, *frame->data + size);
    // for(int i = 0; i < size; ++i)
    // {
    //     rgb[i] = frame->data[i];
    // }
    ColorConvert Convert;
    std::vector<uint8_t> Ret = Convert.YUV422ToRGB24(rgb,  width, height);
    Utils::GetInstance().SavePPM("abc.ppm", Ret, width, height);
    return 0;
}
