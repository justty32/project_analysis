cmake_minimum_required(VERSION 3.10)
project(mylang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Only compile files needed for Parser testing
set(SOURCES 
    src/main.cpp 
    src/lexer.cpp 
    src/parser.cpp 
    src/value.cpp
)

add_executable(mylang ${SOURCES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
