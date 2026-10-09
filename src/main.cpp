#include <functional>
#include <chrono>
#include "VideoRecorder.h"
#include "V4l2Capture.h"
#include "Utils.h"
#include "ColorConvert.h"
#include "Define.h"
#include "Encoder.h"
#include "Muxer.h"
#include "AlsaCapture.h"
#include "VideoRecorder.h"

int main()
{
    VideoRecorder Recorder;
    bool Retval = Recorder.Config({"/dev/video0", "Video.mp4", {0,0,6}});
    if(Retval == false)
    {
        return -1;
    }
    Recorder.Record();

    LOGI("Capture finished");
    return 0;
}
