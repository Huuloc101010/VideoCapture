#include <functional>
#include <chrono>
#include "VideoRecorder.h"
#include "V4l2Capture.h"
#include "Utils.h"
#include "ColorConvert.h"
#include "Define.h"
#include "Encoder.h"
#include "Muxer.h"
#include "AlsaCapture.h"
#include "VideoRecorder.h"

int main()
{
    // VideoRecorder Recorder;
    // Recorder.Config({"/dev/video0", "Video.mp4", {0,0,6}});
    // Recorder.Record();
    AlsaCapture Alsa;
    if(Alsa.Config({"hw:0,0", "48000", "2"}))
    {
        LOGI("Config success");
    }
    AVStream* Stream = Alsa.GetStream();

    LOGI("Sample rate: {}", Stream->codecpar->sample_rate);
    //LOGI("Channels: {}", Stream->codecpar->ch_layout.nb_channels);
    LOGI("Sample format: {}", Stream->codecpar->format);

    std::ofstream File("audio.raw", std::ios::binary);

    if (!File.is_open())
    {
        LOGE("Can not open audio.raw");
        return -1;
    }

    for (int i = 0; i < 200; ++i)
    {
        UniquePacketPtr Packet = Alsa.ReadPacket();

        if (!Packet)
        {
            LOGE("Read packet failed");
            break;
        }

        File.write(
            reinterpret_cast<const char*>(Packet->data),
            Packet->size
        );

        LOGI(
            "Packet {}: size = {} bytes, pts = {}",
            i,
            Packet->size,
            Packet->pts
        );
    }

    File.close();

    LOGI("Capture finished");
    return 0;
}
