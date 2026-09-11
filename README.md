This software provide funtion to capture in v4l2 (Video for linux verion 2)
source code designed use c++20 , ffmpeg library

step 1:
capture raw fame yuyv in /dev/video0
step 2:
Convert color space: yuyv (yuv422) to yuv420
step 3:
encode yuv420 -> h.264 stream
step 4:
pack h.264 stream into .mp4 container


source code file include:
- main
- class VideoRecorder
- class Mp4Muxer
- define.h -> smartpointer for ffmpeg,...
- Video
+ class V4l2Capture
+ class ColorCovert
+ class H264Encoder
- Audio
+ class AudioCapture
+ class AudioConvert
+ class AACEncoder

Ultility

class Log

