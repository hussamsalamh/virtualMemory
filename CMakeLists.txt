cmake_minimum_required(VERSION 3.17)
project(ex4)

set(CMAKE_CXX_STANDARD 14)

add_executable(ex4 main.cpp virtualMemory/VirtualMemory.cpp
        virtualMemory/VirtualMemory.h virtualMemory/SimpleTest.cpp
        virtualMemory/PhysicalMemory.cpp virtualMemory/PhysicalMemory.h)