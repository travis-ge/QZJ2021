cmake_minimum_required(VERSION 3.16)
project(QZJ)

set(CMAKE_CXX_STANDARD 14)

add_executable(QZJ main.cpp)


find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(QZJ ${OpenCV_LIBS})
target_link_libraries(QZJ wiringPi)