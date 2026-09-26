#include "VideoRecorder.h"
#include "Log.h"
#include "Utils.h"
#include "Define.h"


void VideoRecorder::Record()
{
    constexpr int width = 640;
    constexpr int height = 480;
    CaptureType type = CaptureType::V4L2_NATIVE;
    CaptureType type2 = CaptureType::FFMPEG_CAPTURE;
    V4l2CaptureConfig Config = {type2, "/dev/video0", width, height, 30, AV_PIX_FMT_YUYV422};
    LOGW("type {}", (int)type);

    if(m_V4l2Capture.Config(Config) == false)
    {
        LOGE("Config fail");
        return;
    }
    ColorConvertConfig ConvertConfig = {width, height, AV_PIX_FMT_YUYV422};
    
    m_ColorConvert.ConfigColorConvert(ConvertConfig);
    EncoderConfig EncoderConfig = {width, height, MediaType::VIDEO};
    
    if(m_Encoder.ConfigEncoder(EncoderConfig) == false)
    {
        LOGE("Config encoder fail");
        return;
    }
    LOGI(
    "Encoder TB = {}/{}",
    m_Encoder.GetVideoContext()->time_base.num,
    m_Encoder.GetVideoContext()->time_base.den
    );
    LOGI("Config success");
    // FILE* File = fopen("output.h264", "wb");
    // if (File == nullptr)
    // {
    //     LOGE("Cannot open output.h264");
    //     return -1;
    // }
    //std::vector<UniquePacketPtr> Total;
    
    m_Muxer.Config({width, height, m_Encoder.GetVideoContext().get(), nullptr, "mp4", "Video.mp4"});
    m_Muxer.WriteHeader();
    auto Start = std::chrono::steady_clock::now();
    int64_t FirstPts = AV_NOPTS_VALUE;
    for(int i = 0; i < 200; ++i)
    {
        UniquePacketPtr packet = m_V4l2Capture.ReadPacket();
        if(packet == nullptr)
        {
            LOGE("packet is null");
            return;
        }
        // Save first timstamp
        if(FirstPts == AV_NOPTS_VALUE)
        {
            FirstPts = packet->pts;
            packet->pts = 0;
        }
        else
        {
            packet->pts = packet->pts - FirstPts;
        }
        LOGI(
        "Frame {}, pts={}, size={}",
        i,
        packet->pts,
        packet->size
        );
    
        UniqueFramePtr frame = m_ColorConvert.ConvertPacketToFrame(std::move(packet));
        if(frame == nullptr)
        {
            LOGE("frame is nullptr");
            return;
        }
        UniqueFramePtr YUV420 = m_ColorConvert.ConvertYUV422ToYUV420(std::move(frame));
        if(YUV420 == nullptr)
        {
            LOGE("YUV420 is nullptr");
            return;
        }
        LOGW("Frame pts {}", YUV420->pts);
        // Capture TB -> Encoder TB
        YUV420->pts = av_rescale_q(
            YUV420->pts,
            m_V4l2Capture.GetStream()->time_base,
            m_Encoder.GetVideoContext()->time_base
        );
        std::vector<UniquePacketPtr> VectorEncoder;
        if(i == 199)
        {
            VectorEncoder =  m_Encoder.Encode(nullptr);

        }
        else
        {

            VectorEncoder =  m_Encoder.Encode(std::move(YUV420));
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
                m_V4l2Capture.GetStream()->time_base.num,
                m_V4l2Capture.GetStream()->time_base.den
                );
            // Total.emplace_back(std::move(Packet));
            m_Muxer.WritePacket(std::move(Packet));
        }
    }
    auto End = std::chrono::steady_clock::now();

    double Seconds =
    std::chrono::duration<double>(End - Start).count();

    LOGI("100 frames captured in {} seconds", Seconds);
    LOGI("Actual FPS = {}", 100.0 / Seconds);
    m_Muxer.WriteTrailer();
}