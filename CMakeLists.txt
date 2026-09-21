cmake_minimum_required(VERSION 3.16)

project(VideoRecorder)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find all .cpp files under src/
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Add compiler flags for GCC / Clang
add_compile_options(-g3 -fsanitize=address -fno-omit-frame-pointer)

# Add linker flags to ensure ASan runtime is linked properly
add_link_options(-fsanitize=address)

add_executable(VideoRecorder ${SOURCES})

# Find FFmpeg
find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG REQUIRED
    libavformat
    libavcodec
    libavutil
    libavdevice
    libswscale
)

# Header files
target_include_directories(VideoRecorder PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Utils
    ${CMAKE_SOURCE_DIR}/src/Video
    ${FFMPEG_INCLUDE_DIRS}
)

# Library directories
target_link_directories(VideoRecorder PRIVATE
    ${FFMPEG_LIBRARY_DIRS}
)

# Link FFmpeg
target_link_libraries(VideoRecorder PRIVATE
    ${FFMPEG_LIBRARIES}
)
