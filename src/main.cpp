#include <functional>
#include <chrono>
#include "VideoRecorder.h"
#include "V4l2Capture.h"
#include "Utils.h"
#include "ColorConvert.h"
#include "Define.h"
#include "Encoder.h"
#include "Muxer.h"

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
    ColorConvertConfig ConvertConfig = {width, height, AV_PIX_FMT_YUYV422};
    ColorConvert Convert(ConvertConfig);
    EncoderConfig EncoderConfig = {width, height, MediaType::VIDEO};
    Encoder encoder;
    if(encoder.ConfigEncoder(EncoderConfig) == false)
    {
        return false;
    }
    LOGI(
    "Encoder TB = {}/{}",
    encoder.GetVideoContext()->time_base.num,
    encoder.GetVideoContext()->time_base.den
    );
    LOGI("Config success");
    FILE* File = fopen("output.h264", "wb");
    if (File == nullptr)
    {
        LOGE("Cannot open output.h264");
        return -1;
    }
    //std::vector<UniquePacketPtr> Total;
    Muxer Mux;
    Mux.Config({width, height, encoder.GetVideoContext().get(), nullptr, "mp4", "Video.mp4"});
    Mux.WriteHeader();
    auto Start = std::chrono::steady_clock::now();
    int64_t FirstPts = AV_NOPTS_VALUE;
    for(int i = 0; i < 1000; ++i)
    {
        
        UniquePacketPtr packet = Capture.ReadPacket();
        if(packet == nullptr)
        {
            LOGE("packet is null");
            return -1;
        }
        LOGI(
        "Frame {}, pts={}, size={}",
        i,
        packet->pts,
        packet->size
        );
    
        UniqueFramePtr frame = Convert.ConvertPacketToFrame(std::move(packet));
        if(frame == nullptr)
        {
            LOGE("frame is nullptr");
            return -1;
        }
        UniqueFramePtr YUV420 = Convert.ConvertYUV422ToYUV420(std::move(frame));
        if (FirstPts == AV_NOPTS_VALUE)
        {
            FirstPts = YUV420->pts;
        }
        if(YUV420 == nullptr)
        {
            LOGE("YUV420 is nullptr");
            return -1;
        }
        // Capture TB -> Encoder TB
        YUV420->pts = av_rescale_q(
            YUV420->pts,
            Capture.GetStream()->time_base,
            encoder.GetVideoContext()->time_base
        );
        std::vector<UniquePacketPtr> VectorEncoder;
        if(i == 999)
        {
            VectorEncoder =  encoder.Encode(nullptr);

        }
        else
        {

            VectorEncoder =  encoder.Encode(std::move(YUV420));
        }
        LOGE("VectorEncoder.size()={}", VectorEncoder.size());
        
        for (auto& Packet : VectorEncoder)
        {
            if (Packet == nullptr)
            {
                LOGE("Packet is nullptr");
                continue;
            }

            // size_t Written = fwrite(
            //     Packet->data,
            //     1,
            //     Packet->size,
            //     File
            // );

            // if (Written != static_cast<size_t>(Packet->size))
            // {
            //     LOGE("fwrite failed");
            //     fclose(File);
            //     return -1;
            // }

           LOGI("Packet pts={}, dts={}, duration={}, time_base={}/{}",
                Packet->pts,
                Packet->dts,
                Packet->duration,
                Capture.GetStream()->time_base.num,
                Capture.GetStream()->time_base.den
                );
            // Total.emplace_back(std::move(Packet));
            Mux.WritePacket(std::move(Packet));
        }
    }
    auto End = std::chrono::steady_clock::now();

    double Seconds =
    std::chrono::duration<double>(End - Start).count();

    LOGI("100 frames captured in {} seconds", Seconds);
    LOGI("Actual FPS = {}", 100.0 / Seconds);
    //MuxerConfig MuxConfig = {width, height, std::ref(encoder.GetVideoContext()), nullptr, "mp4", "Video.mp4"};
    
    // for(auto& Packet : Total)
    // {

        
    // }
    Mux.WriteTrailer();
    //fclose(File);

    return 0; 
}
