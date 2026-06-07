cmake_minimum_required(VERSION 3.16)

project(TrafficSafetyAI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(traffic_safety_ai
    src/main.cpp
)

if(MSVC)
    target_compile_options(traffic_safety_ai PRIVATE /W4)
else()
    target_compile_options(traffic_safety_ai PRIVATE -Wall -Wextra -pedantic)
endif()
