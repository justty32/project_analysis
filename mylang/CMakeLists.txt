cmake_minimum_required(VERSION 3.10)
project(mylang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Automatically find all source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(mylang ${SOURCES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
