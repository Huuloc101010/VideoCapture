#include <thread>
#include "VideoRecorder.h"
#include "Log.h"
#include "Utils.h"
#include "Define.h"

/*
void VideoRecorder::Record()
{
    m_IsRunning = true;
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
            // Reset present timestamp
            packet->pts = packet->pts - FirstPts;
        }
        LOGI("Frame {}, pts={}, size={}", FrameCount, packet->pts, packet->size);
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
        Utils::GetInstance().ConvertTimestamp(YUV420, m_V4l2Capture.GetTimeBase(), m_VideoEncoder.GetTimeBase());
        std::vector<UniquePacketPtr>  VectorEncoder =  m_VideoEncoder.Encode(std::move(YUV420));
        // LOGI("VectorEncoder.size()={}", VectorEncoder.size());
        
        for (auto& Packet : VectorEncoder)
        {
            if (Packet == nullptr)
            {
                LOGE("Packet is nullptr");
                continue;
            }

        Utils::GetInstance().ConvertTimestamp(Packet, m_VideoEncoder.GetTimeBase(), m_Muxer.GetTimeBase());
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
*/

void VideoRecorder::Record()
{
    m_IsRunning = true;
    m_Muxer.WriteHeader();
    auto Start = std::chrono::steady_clock::now();
    std::jthread CheckTime(&VideoRecorder::CheckTimeRecorded, this);
    int FrameCount = 0;
    int64_t FirstTimeStamp = AV_NOPTS_VALUE;
    //while(m_IsRunning == true)
    for(int i = 0; i <= 100; ++i)
    {
        UniquePacketPtr Packet = m_AlsaCapture.ReadPacket();
        if(Packet == nullptr)
        {
            LOGE("Packet is nullptr");
            return;
        }
        // Save first timestamp
        if(FirstTimeStamp == AV_NOPTS_VALUE)
        {
            FirstTimeStamp = Packet->pts;
        }
        // Reset present timstamp
        Packet->pts = Packet->pts - FirstTimeStamp;

        UniqueFramePtr FrameS16P = m_AudioConvert.ConvertPacketToFrame(std::move(Packet));
        if(FrameS16P == nullptr)
        {
            LOGE("FrameS16P = nullptr");
            return;
        }

        UniqueFramePtr FrameFPTP = m_AudioConvert.ConvertS16ToFPTP(std::move(FrameS16P));
        if(FrameFPTP == nullptr)
        {
            LOGE("FrameFPTP = nullptr");
            return;
        }
        // Split frame 4096 to 1024
        std::vector<UniqueFramePtr> VectorFrame = m_AudioConvert.SplitPacket(std::move(FrameFPTP));
        LOGE("Size VectorFrame = {}", VectorFrame.size());
        for(auto& Element : VectorFrame)
        {
            LOGW("Duration {}", Element->pkt_duration);
            LOGW("Timestamp pts {}", Element->pts);
            Utils::GetInstance().ConvertTimestamp(Element, m_AlsaCapture.GetTimeBase(), m_AudioEncoder.GetTimeBase());
            std::vector<UniquePacketPtr> VectorPacket = {};
            if(i < 100)
            {
                VectorPacket = m_AudioEncoder.Encode(std::move(Element));
            }
            else
            {
                VectorPacket = m_AudioEncoder.Encode(nullptr);
            }
            for(auto& SubElement : VectorPacket)
            {
                Utils::GetInstance().ConvertTimestamp(SubElement, m_AudioEncoder.GetTimeBase(), m_Muxer.GetTimeBase());
                Utils::GetInstance().PrintTimeStamp(m_Muxer.GetTimeBase(), SubElement->pts);
                LOGE("Packet->pts = {}", SubElement->pts);
                m_Muxer.WritePacket(std::move(SubElement));
            }
        }
    }
    // Flush encoder
    auto End = std::chrono::steady_clock::now();
    //FlushEncoder();
    double Seconds =
    std::chrono::duration<double>(End - Start).count();

    LOGI("100 frames captured in {} seconds", Seconds);
    LOGI("Actual FPS = {}", FrameCount / Seconds);
    m_Muxer.WriteTrailer();
}

bool VideoRecorder::Config(const VideoRecorderConfig& Config)
{
    constexpr int width = 640;
    constexpr int height = 480;
    // Calculate time
    m_Config = Config;
    m_TimeRecord = 0;
    m_TimeRecord += m_Config.Time.Hour    * 60 * 60;
    m_TimeRecord += m_Config.Time.Minutes * 60;
    m_TimeRecord += m_Config.Time.Second;
    if(ConfigVideo() == false)
    {
        LOGE("Config Video fail");
        return false;
    }
    if(ConfigAudio() == false)
    {
        LOGE("Config Audio fail");
        return false;
    }

    if(m_Muxer.Config({width, height, m_VideoEncoder.GetCodecContext().get(), m_AudioEncoder.GetCodecContext().get(), "mp4", m_Config.PathVideoOutput}) == false)
    {
        LOGE("Config muxer fail");
        return false;
    }
    return true;
}

bool VideoRecorder::ConfigVideo()
{
    constexpr int width = 640;
    constexpr int height = 480;
    CaptureType type = CaptureType::V4L2_NATIVE;
    CaptureType type2 = CaptureType::FFMPEG_CAPTURE;
    V4l2CaptureConfig Config = {type2, m_Config.PathDevice, width, height, 30, AV_PIX_FMT_YUYV422};
    LOGW("type {}", (int)type);

    if(m_V4l2Capture.Config(Config) == false)
    {
        LOGE("Config fail");
        return false;
    }
    ColorConvertConfig ConvertConfig = {width, height, AV_PIX_FMT_YUYV422};
    
    m_ColorConvert.ConfigColorConvert(ConvertConfig);
    EncoderConfig EncoderConfig = {width, height, MediaType::VIDEO};
    
    if(m_VideoEncoder.ConfigEncoder(EncoderConfig) == false)
    {
        LOGE("Config encoder fail");
        return false;
    }
    LOGI("Config success");

    return true;
}

bool VideoRecorder::ConfigAudio()
{
    constexpr int width = 640;
    constexpr int height = 480;
    if(m_AlsaCapture.Config({"hw:0,0", "48000", "2"}) == false)
    {
        LOGI("Config alsa fail");
        return false;
    }
    AVStream* Stream = m_AlsaCapture.GetStream();
    if(Stream == nullptr)
    {
        LOGE("Stream alsa null");
        return false;
    }
    LOGI("Sample rate: {}", Stream->codecpar->sample_rate);
    //LOGI("Channels: {}", Stream->codecpar->ch_layout.nb_channels);
    LOGI("Sample format: {}", Stream->codecpar->format);
    EncoderConfig EncoderConfig = {width, height, MediaType::AUDIO};
    if(m_AudioEncoder.ConfigEncoder(EncoderConfig) == false)
    {
        LOGE("Config encoder fail");
        return false;
    }

    AudioConvertConfig Config = {m_AlsaCapture.GetStream(), 48000, AV_CH_LAYOUT_STEREO};
    if(m_AudioConvert.Config(Config) == false)
    {
        LOGE("Audio convert fail");
        return false;
    }

    return true;
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
    std::vector<UniquePacketPtr> VectorEncoder = m_VideoEncoder.Encode(nullptr);
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