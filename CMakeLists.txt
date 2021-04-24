cmake_minimum_required(VERSION 3.19)
project(zad1cpp)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "-g -Wall -Wextra std=c++17")


add_executable(zad1cpp serwer.cpp
        correlatedServer.cpp correlatedServer.h
        parser.cpp parser.h
        responses.cpp responses.h
        requests.cpp requests.h
        httpExceptions.h)

add_executable(test test.cpp
        parser.cpp parser.h)
