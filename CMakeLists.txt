cmake_minimum_required(VERSION 3.19)
project(zad1cpp)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "-g -Wall -Wextra")


add_executable(zad1cpp server.cpp CorrelatedServer.h)