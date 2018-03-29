cmake_minimum_required(VERSION 3.9)
project(newTDHH)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost 1.58.0 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_executable(newTDHH src/main.cpp src/PacketsReader.cpp src/PacketsReader.hpp src/IPPacket.cpp src/IPPacket.hpp src/CSVIterator.cpp src/CSVIterator.hpp src/Router.cpp src/Router.hpp src/QMax.cpp src/QMax.hpp src/Hyperloglog.hpp src/MurmurHash3.hpp src/MurmurHash3.cpp)
target_link_libraries(newTDHH ${Boost_LIBRARIES})

