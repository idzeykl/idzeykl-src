cmake_minimum_required(VERSION 3.10)

project(IdzeyKL)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(idzeykl
        #src/main/mainRedirectedBuffer.cpp
        src/main/mainClassicBuffer.cpp
        src/lexer/lexer.cpp
        src/parser/parser.cpp
        src/interpreter/interpreter.cpp
        src/main/BufferFunc.hpp)
