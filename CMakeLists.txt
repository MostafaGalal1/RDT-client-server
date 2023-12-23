cmake_minimum_required(VERSION 3.26)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

add_executable(server
        server.cpp
        packets.h
        check_sum.cpp
        packet_construction.cpp
)

add_executable(client
        client.cpp
        packets.h
        check_sum.cpp
        packet_construction.cpp
)

