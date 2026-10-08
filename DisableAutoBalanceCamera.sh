#!/bin/bash

v4l2-ctl -d /dev/video0 --set-ctrl=exposure_dynamic_framerate=0

v4l2-ctl --set-fmt-video=width=640,height=480,pixelformat=YUYV --stream-mmap  -d /dev/video0