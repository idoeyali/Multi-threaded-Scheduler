cmake_minimum_required(VERSION 3.16)
project(Multi_Threads_Programming)

set(CMAKE_CXX_STANDARD 11)

add_executable(Multi_Threads_Programming
        main.cpp
        Scheduler.cpp
        Scheduler.h
        Thread.cpp
        Thread.h
        uthreads.cpp
        uthreads.h)
