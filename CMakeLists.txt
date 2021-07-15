cmake_minimum_required(VERSION 3.0)

project(EASYPROGRAM)

SET(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++11 -pthread")

include_directories(./include)

add_executable(demo src/demo.cpp src/HTTPSession.cpp src/utils.cpp src/ThreadPool.cpp)