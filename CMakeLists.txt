cmake_minimum_required(VERSION 3.0)

project(buzzer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

add_executable(buzzer
    src/main.cpp
)