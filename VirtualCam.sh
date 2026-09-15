sudo modprobe v4l2loopback video_nr=2 card_label="VirtualCam" exclusive_caps=1

ffmpeg -stream_loop -1 -re -i ./VideoSample/video.mp4 -vf "scale=640:480,format=yuyv422" -f v4l2 /dev/video2