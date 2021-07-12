cmake_minimum_required(VERSION 3.0)

project(EASYPROGRAM)

SET(CMAKE_BUILD_TYPE "Debug")

include_directories(./include)


add_executable(demo src/demo.cpp)