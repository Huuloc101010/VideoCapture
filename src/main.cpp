#include <functional>
#include <csignal>
#include <unistd.h>
#include "VideoRecorder.h"
#include "V4l2Capture.h"
#include "Utils.h"
#include "ColorConvert.h"
#include "Define.h"
#include "Encoder.h"
#include "Muxer.h"
#include "AlsaCapture.h"
#include "VideoRecorder.h"

// Use global variable for signal system
std::function<void()> Callback;
std::mutex MutexHandle;
void SignalHandler(int Signal)
{
    std::lock_guard<std::mutex> Lock(MutexHandle);
    LOGW("Received signal stop");
    if(Callback)
    {
        Callback();
        Callback = nullptr;
    }
}

int main()
{
    VideoRecorder Recorder(FILE_MP4);
    Callback = [&]()
    {
        Recorder.Stop();
    };
    VideoRecorderConfig RecorderConfig;
    RecorderConfig.PathDevice = "/dev/video0";
    RecorderConfig.PathVideoOutput = "video.mp4";
    RecorderConfig.TailVideo = "mp4";
    RecorderConfig.Width = 640;
    RecorderConfig.Height = 480;
    RecorderConfig.PathAudioDevice = "hw:0,0";
    RecorderConfig.AudioSampleRate = "48000";
    RecorderConfig.AudioSampleChannel = "2";
    RecorderConfig.Time = {0,0,10};
    
    bool Retval = Recorder.Config(RecorderConfig);
    if(Retval == false)
    {
        return -1;
    }
    // Register to system
    std::signal(SIGINT, SignalHandler);
    Recorder.Record();

    LOGI("Capture finished");
    return 0;
}
