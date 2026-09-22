#include "VideoRecorder.h"
#include "V4l2Capture.h"
#include "Utils.h"
#include "ColorConvert.h"
#include "Define.h"
#include "Encoder.h"

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
    ColorConvertConfig ConvertConfig = {width, height, AV_PIX_FMT_YUYV422};
    ColorConvert Convert(ConvertConfig);
    EncoderConfig EncoderConfig = {width, height, MediaType::VIDEO};
    Encoder encoder;
    if(encoder.ConfigEncoder(EncoderConfig) == false)
    {
        return false;
    }
    LOGI("Config success");
    FILE* File = fopen("output.h264", "wb");
    if (File == nullptr)
    {
        LOGE("Cannot open output.h264");
        return -1;
    }
    for(int i = 0; i < 1000; ++i)
    {
        
        UniquePacketPtr packet = Capture.ReadPacket();
        if(packet == nullptr)
        {
            LOGE("packet is null");
            return -1;
        }
        
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

        std::vector<UniquePacketPtr> VectorEncoder =  encoder.Encode(std::move(YUV420));
        LOGE("VectorEncoder.size()={}", VectorEncoder.size());
        for (auto& Packet : VectorEncoder)
        {
            if (Packet == nullptr)
            {
                LOGE("Packet is nullptr");
                continue;
            }

            size_t Written = fwrite(
                Packet->data,
                1,
                Packet->size,
                File
            );

            if (Written != static_cast<size_t>(Packet->size))
            {
                LOGE("fwrite failed");
                fclose(File);
                return -1;
            }

            LOGI(
                "Write packet: size={}, pts={}, dts={}",
                Packet->size,
                Packet->pts,
                Packet->dts
            );
        }
    }
    fclose(File);

    return 0; 
}
