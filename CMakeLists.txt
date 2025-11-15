cmake_minimum_required(VERSION 3.31)
project(argc__)

set(CMAKE_CXX_STANDARD 20)

add_executable(argc__ test/main.cpp)

include_directories(
        SYSTEM
        ${CMAKE_SOURCE_DIR}/src/argc--/)