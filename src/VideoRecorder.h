#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H

#include <iostream>
#include <vector>
#include <string_view>
#include <memory>
#include <algorithm>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "V4l2Capture.h"
#include "Encoder.h"
#include "Muxer.h"
#include "ColorConvert.h"
#include "AlsaCapture.h"
#include "AudioConvert.h"

class VideoRecorder
{
public:
    VideoRecorder() = default;
    ~VideoRecorder() = default;
    void Config(const VideoRecorderConfig& Config);
    bool ConfigVideo();
    bool ConfigAudio();
    void Record();
    void Stop();
private:
    void CheckTimeRecorded();
    void FlushEncoder();
    V4l2Capture          m_V4l2Capture;
    Encoder              m_Encoder;
    Muxer                m_Muxer;
    ColorConvert         m_ColorConvert;
    VideoRecorderConfig  m_Config;
    std::atomic<uint64_t> m_TimeRecord;
    std::atomic<bool>     m_IsRunning;
    AlsaCapture           m_AlsaCapture;
    AudioConvert          m_AudioConvert;
};

#endif // VIDEO_RECORDER_H