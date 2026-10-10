#include <thread>
#include "VideoRecorder.h"
#include "Log.h"
#include "Utils.h"
#include "Define.h"

VideoRecorder::VideoRecorder()
{
    m_FlushSuccessCounter = 0;
}

void VideoRecorder::Record()
{
    m_IsRunning = true;
    m_Muxer.WriteHeader();
    Clock Clock;
    int64_t FirstPts = AV_NOPTS_VALUE;
    int FrameCount = 0;
    std::jthread CheckTime(&VideoRecorder::CheckTimeRecorded, this);
    std::jthread ThreadVideo(&VideoRecorder::ThreadCaptureVideo, this);
    std::jthread TheadAudio(&VideoRecorder::ThreadCaptureAudio, this);
    std::jthread ThreadWriteFrame(&VideoRecorder::ThreadWriteFrame, this);
    ThreadVideo.join();
    TheadAudio.join();
    // Flush video and audio encoder
    FlushVideoEncoder();
    FlushAudioEncoder();
    ThreadWriteFrame.join();

    LOGI("Captured in {} seconds", Clock.GetTimeSeconds());
    m_Muxer.WriteTrailer();
    m_IsRunning = false;
}

bool VideoRecorder::Config(const VideoRecorderConfig& Config)
{
    const int width = Config.Width;
    const int height = Config.Height;
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

    if(m_Muxer.Config({width, height, m_VideoEncoder.GetCodecContext().get(), m_AudioEncoder.GetCodecContext().get(), m_Config.TailVideo, m_Config.PathVideoOutput}) == false)
    {
        LOGE("Config muxer fail");
        return false;
    }
    return true;
}

bool VideoRecorder::ConfigVideo()
{
    const int width = m_Config.Width;
    const int height = m_Config.Height;
    CaptureType type = CaptureType::V4L2_NATIVE;
    CaptureType type2 = CaptureType::FFMPEG_CAPTURE;
    V4l2CaptureConfig Config = {type2, m_Config.PathDevice, width, height, 30, AV_PIX_FMT_YUYV422};

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
    const int width = m_Config.Width;
    const int height = m_Config.Height;
    if(m_AlsaCapture.Config({m_Config.PathAudioDevice, m_Config.AudioSampleRate, m_Config.AudioSampleChannel}) == false)
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
    EncoderConfig EncoderConfig = {width, height, MediaType::AUDIO};
    if(m_AudioEncoder.ConfigEncoder(EncoderConfig) == false)
    {
        LOGE("Config encoder fail");
        return false;
    }

    AudioConvertConfig Config = {m_AlsaCapture.GetStream(), std::stoi(m_Config.AudioSampleRate), AV_CH_LAYOUT_STEREO};
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void VideoRecorder::FlushVideoEncoder()
{
    std::vector<UniquePacketPtr> VectorEncoder = m_VideoEncoder.Encode(nullptr);
    for (auto& Packet : VectorEncoder)
    {
        if (Packet == nullptr)
        {
            LOGE("Packet is nullptr");
            continue;
        }
        Utils::GetInstance().ConvertTimestamp(Packet, m_VideoEncoder.GetTimeBase(), m_Muxer.GetVideoTimeBase());
        m_VideoQueue.Push(std::move(Packet));
    }
    m_FlushSuccessCounter ++;
    LOGI("Flush video encoder success");
}

void VideoRecorder::FlushAudioEncoder()
{
    std::vector<UniquePacketPtr> VectorEncoder = m_AudioEncoder.Encode(nullptr);
    for (auto& Packet : VectorEncoder)
    {
        if(Packet == nullptr)
        {
            LOGE("Packet is nullptr");
            continue;
        }
        Utils::GetInstance().ConvertTimestamp(Packet, m_VideoEncoder.GetTimeBase(), m_Muxer.GetVideoTimeBase());
        m_AudioQueue.Push(std::move(Packet));
    }
    m_FlushSuccessCounter ++;
    LOGI("Flush audio encoder success");
}

void VideoRecorder::Stop()
{
    m_IsRunning = false;
}

void VideoRecorder::ThreadCaptureVideo()
{
    Clock Clock;
    while(m_IsRunning == true)
    {
        UniquePacketPtr packet = m_V4l2Capture.ReadPacket();
        if(packet == nullptr)
        {
            LOGE("packet is null");
            continue;
        }
        // Recalculate timestamp follow system clock
        packet->pts = Utils::GetInstance().GetTimeStamp(m_V4l2Capture.GetTimeBase(), Clock.GetTimeSeconds());
        LOGW("Thread video: {}", Clock.GetTimeSeconds());
        UniqueFramePtr frame = m_ColorConvert.ConvertPacketToFrame(std::move(packet));
        if(frame == nullptr)
        {
            LOGE("frame is nullptr");
            continue;
        }
        UniqueFramePtr YUV420 = m_ColorConvert.ConvertYUV422ToYUV420(std::move(frame));
        if(YUV420 == nullptr)
        {
            LOGE("YUV420 is nullptr");
            continue;
        }
        // Capture TB -> Encoder TB
        Utils::GetInstance().ConvertTimestamp(YUV420, m_V4l2Capture.GetTimeBase(), m_VideoEncoder.GetTimeBase());
        std::vector<UniquePacketPtr>  VectorEncoder =  m_VideoEncoder.Encode(std::move(YUV420));

        for (auto& Packet : VectorEncoder)
        {
            if (Packet == nullptr)
            {
                LOGE("Packet is nullptr");
                continue;
            }
            Utils::GetInstance().ConvertTimestamp(Packet, m_VideoEncoder.GetTimeBase(), m_Muxer.GetVideoTimeBase());
            m_VideoQueue.Push(std::move(Packet));
        }
    }
}

void VideoRecorder::ThreadCaptureAudio()
{
    Clock Clock;
    while(m_IsRunning == true)
    {
        UniquePacketPtr Packet = m_AlsaCapture.ReadPacket();
        if(Packet == nullptr)
        {
            LOGE("Packet is nullptr");
            continue;
        }
        Packet->pts = Utils::GetInstance().GetTimeStamp(m_AlsaCapture.GetTimeBase(), Clock.GetTimeSeconds());
        LOGE("Thread audio: {}", Clock.GetTimeSeconds());
        UniqueFramePtr FrameS16P = m_AudioConvert.ConvertPacketToFrame(std::move(Packet));
        if(FrameS16P == nullptr)
        {
            LOGE("FrameS16P == nullptr");
            continue;
        }

        UniqueFramePtr FrameFPTP = m_AudioConvert.ConvertS16ToFPTP(std::move(FrameS16P));
        if(FrameFPTP == nullptr)
        {
            LOGE("FrameFPTP = nullptr");
            continue;
        }
        // Split frame 4096 to 1024
        std::vector<UniqueFramePtr> VectorFrame = m_AudioConvert.SplitPacket(std::move(FrameFPTP));
        for(auto& Element : VectorFrame)
        {
            Utils::GetInstance().ConvertTimestamp(Element, m_AlsaCapture.GetTimeBase(), m_AudioEncoder.GetTimeBase());
            std::vector<UniquePacketPtr> VectorPacket = m_AudioEncoder.Encode(std::move(Element));
            for(auto& SubElement : VectorPacket)
            {
                Utils::GetInstance().ConvertTimestamp(SubElement, m_AudioEncoder.GetTimeBase(), m_Muxer.GetAudioTimeBase());
                m_AudioQueue.Push(std::move(SubElement));
            }
        }
    }
}

void VideoRecorder::ThreadWriteFrame()
{
    UniquePacketPtr VideoPacket = nullptr;
    UniquePacketPtr AudioPacket = nullptr;
    while((m_IsRunning == true) || (m_FlushSuccessCounter != 2)) // 2 is counter for do flush video and audio success
    {
        int64_t VideoPts = INT64_MAX;
        int64_t AudioPts = INT64_MAX;
        if(m_VideoQueue.Size() && (VideoPacket == nullptr))
        {
            VideoPacket = m_VideoQueue.Pop();
        }
        if(m_AudioQueue.Size() && (AudioPacket == nullptr))
        {
            AudioPacket = m_AudioQueue.Pop();
        }
        if((VideoPacket == nullptr) && (AudioPacket == nullptr))
        {
            // Skip if 2 queue pop fail (Maybe queue stoped)
            continue;
        }
        if(VideoPacket != nullptr)
        {
            VideoPts = VideoPacket->pts;
        }
        if(AudioPacket != nullptr)
        {
            AudioPts = AudioPacket->pts;
        }
        if(VideoPts < AudioPts)
        {
            // Write video
            m_Muxer.WriteVideoPacket(std::move(VideoPacket));
        }
        else
        {
            // Write audio
            m_Muxer.WriteAudioPacket(std::move(AudioPacket));
        }
    }
}
