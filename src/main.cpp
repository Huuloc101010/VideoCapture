#include "VideoRecorder.h"
#include "V4l2Capture.h"


int main()
{
	
    VideoRecorder Record;
    Record.Capture();
/*
    constexpr int width = 640;
    constexpr int height = 480;
    V4l2Capture Capture;
    Capture.Start("/dev/video2", width, height, 30);
    Capture.start();
    Capture.read_packet();
    */
    return 0;
}
