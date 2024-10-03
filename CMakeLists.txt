cmake_minimum_required(VERSION 3.30)
project(demo)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
message("opencv: ${OpenCV_INCLUDE_DIRS}")
find_package(FFmpeg REQUIRED)
message("opencv: ${FFmpeg_INCLUDE_DIRS}")

include_directories(${OpenCV_INCLUDE_DIRS} ${FFmpeg_INCLUDE_DIRS} )

add_executable(demo main.cpp)

target_link_libraries(demo ${OpenCV_LIBS} ${FFmpeg_LIBRARIES})
