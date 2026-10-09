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

void SignalHandler(int Signal)
{
    LOGW("Received signal stop");
    if(Callback)
    {
        Callback();
    }
}

int main()
{
    VideoRecorder Recorder;
    Callback = [&]()
    {
        Recorder.Stop();
    };
    // Register to system
    std::signal(SIGINT, SignalHandler);
    VideoRecorderConfig RecorderConfig;
    RecorderConfig.PathDevice = "/dev/video0";
    RecorderConfig.PathVideoOutput = "Video.mp4";
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
    Recorder.Record();

    LOGI("Capture finished");
    return 0;
}
