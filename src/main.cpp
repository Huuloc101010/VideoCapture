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
    V4l2CaptureConfig Config = {"/dev/video0", width, height, 30};
    Capture.Config(Config);
    Capture.start();
    AVPacket* packet = av_packet_alloc();
    Capture.read_packet(&packet);
    std::cout << packet->size << std::endl;
    std::vector<uint8_t> rgb(packet->size);
    for(int i = 0; i < packet->size; ++i)
    {
        rgb[i] = packet->data[i];
    }
    ColorConvert Convert;
    std::vector<uint8_t> Ret = Convert.YUV422ToRGB24(rgb,  width, height);
    Utils::GetInstance().SavePPM("abc", Ret, width, height);
    return 0;
}
