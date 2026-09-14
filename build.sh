shopt -s globstar
mkdir -p build
g++-13 -std=c++20 **/*.cpp -o ./build/VideoRecorder -I. -I./* -I./src -I./src/Video -I./src/Utils \
`pkg-config --cflags --libs libavformat libavutil libavdevice`