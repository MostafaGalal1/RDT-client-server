cmake_minimum_required(VERSION 3.26)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

add_executable(server
        server.cpp
        cmake-build-debug/packets.h)

add_executable(client
        client.cpp
        cmake-build-debug/packets.h)

