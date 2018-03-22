cmake_minimum_required(VERSION 3.9)
project(newTDHH)

set(CMAKE_CXX_STANDARD 11)

add_executable(newTDHH main.cpp PacketsReader.cpp PacketsReader.hpp IPPacket.cpp IPPacket.hpp CSVIterator.cpp CSVIterator.hpp)