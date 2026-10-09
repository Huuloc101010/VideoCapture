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
    bool Retval = Recorder.Config({"/dev/video0", "Video.mp4", {0,0,10}});
    if(Retval == false)
    {
        return -1;
    }
    Recorder.Record();

    LOGI("Capture finished");
    return 0;
}
