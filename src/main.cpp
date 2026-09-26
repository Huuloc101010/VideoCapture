#include <functional>
#include <chrono>
#include "VideoRecorder.h"
#include "V4l2Capture.h"
#include "Utils.h"
#include "ColorConvert.h"
#include "Define.h"
#include "Encoder.h"
#include "Muxer.h"
#include "VideoRecorder.h"

int main()
{
    VideoRecorder Recorder;
    Recorder.Config({"/dev/video0", "Video.mp4", {0,0,6}});
    Recorder.Record();
    return 0;
}
