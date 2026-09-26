#include <thread>
#include "VideoRecorder.h"
#include "Log.h"
#include "Utils.h"
#include "Define.h"


void VideoRecorder::Record()
{
    m_IsRunning = true;
    constexpr int width = 640;
    constexpr int height = 480;
    CaptureType type = CaptureType::V4L2_NATIVE;
    CaptureType type2 = CaptureType::FFMPEG_CAPTURE;
    V4l2CaptureConfig Config = {type2, m_Config.PathDevice, width, height, 30, AV_PIX_FMT_YUYV422};
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

    m_Muxer.Config({width, height, m_Encoder.GetVideoContext().get(), nullptr, "mp4", m_Config.PathVideoOutput});
    m_Muxer.WriteHeader();
    auto Start = std::chrono::steady_clock::now();
    int64_t FirstPts = AV_NOPTS_VALUE;
    std::jthread CheckTime(&VideoRecorder::CheckTimeRecorded, this);
    int FrameCount = 0;
    while(m_IsRunning == true)
    {
        UniquePacketPtr packet = m_V4l2Capture.ReadPacket();
        FrameCount ++;
        if(packet == nullptr)
        {
            LOGE("packet is null");
            break;
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
        "Frame {}, pts={}, size={}", FrameCount, packet->pts, packet->size);
        UniqueFramePtr frame = m_ColorConvert.ConvertPacketToFrame(std::move(packet));
        if(frame == nullptr)
        {
            LOGE("frame is nullptr");
            break;
        }
        UniqueFramePtr YUV420 = m_ColorConvert.ConvertYUV422ToYUV420(std::move(frame));
        if(YUV420 == nullptr)
        {
            LOGE("YUV420 is nullptr");
            break;
        }
        // LOGI("Frame pts {}", YUV420->pts);
        // Capture TB -> Encoder TB
        YUV420->pts = av_rescale_q(
            YUV420->pts,
            m_V4l2Capture.GetStream()->time_base,
            m_Encoder.GetVideoContext()->time_base
        );
        std::vector<UniquePacketPtr>  VectorEncoder =  m_Encoder.Encode(std::move(YUV420));
        // LOGI("VectorEncoder.size()={}", VectorEncoder.size());
        
        for (auto& Packet : VectorEncoder)
        {
            if (Packet == nullptr)
            {
                LOGE("Packet is nullptr");
                continue;
            }

        //    LOGI("Packet pts={}, dts={}, duration={}, time_base={}/{}",
        //         Packet->pts,
        //         Packet->dts,
        //         Packet->duration,
        //         m_V4l2Capture.GetStream()->time_base.num,
        //         m_V4l2Capture.GetStream()->time_base.den
        //         );
            m_Muxer.WritePacket(std::move(Packet));
        }
    }
    // Flush encoder
    auto End = std::chrono::steady_clock::now();
    FlushEncoder();
    double Seconds =
    std::chrono::duration<double>(End - Start).count();

    LOGI("100 frames captured in {} seconds", Seconds);
    LOGI("Actual FPS = {}", FrameCount / Seconds);
    m_Muxer.WriteTrailer();
}

void VideoRecorder::Config(const VideoRecorderConfig& Config)
{
    m_Config = Config;
    m_TimeRecord = 0;
    m_TimeRecord += m_Config.Time.Hour    * 60 * 60;
    m_TimeRecord += m_Config.Time.Minutes * 60;
    m_TimeRecord += m_Config.Time.Second;
}

void VideoRecorder::CheckTimeRecorded()
{
    auto Start = std::chrono::steady_clock::now();
    while(m_IsRunning == true)
    {
        auto Current = std::chrono::steady_clock::now();
        auto Elapsed = std::chrono::duration_cast<std::chrono::seconds>(Current - Start).count();
        if(Elapsed > m_TimeRecord)
        {
            LOGW("Record timeout");
            m_IsRunning = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void VideoRecorder::FlushEncoder()
{
    std::vector<UniquePacketPtr> VectorEncoder = m_Encoder.Encode(nullptr);
    for (auto& Packet : VectorEncoder)
    {
        if (Packet == nullptr)
        {
            LOGE("Packet is nullptr");
            continue;
        }


        LOGI("Packet pts={}, dts={}, duration={}, time_base={}/{}",
            Packet->pts,
            Packet->dts,
            Packet->duration,
            m_V4l2Capture.GetStream()->time_base.num,
            m_V4l2Capture.GetStream()->time_base.den
            );
        m_Muxer.WritePacket(std::move(Packet));
    }
    LOGI("Flush encoder success");
}

void VideoRecorder::Stop()
{
    m_IsRunning = false;
}