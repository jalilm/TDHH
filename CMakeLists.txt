cmake_minimum_required(VERSION 3.9)
project(TDHH)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost 1.58.0 COMPONENTS system filesystem REQUIRED serialization)
include_directories(${Boost_INCLUDE_DIRS})

set(qmax_LIBRARIES /home/jalilm/CLionProjects/QMAX/cmake-build-debug/libqmax.a)
set(qmax_INCLUDE_DIRS ../QMAX/src)
include_directories(${qmax_INCLUDE_DIRS})

add_executable(TDHH src/main.cpp src/PacketsReader.cpp src/PacketsReader.hpp src/Packet.hpp src/CSVIterator.hpp src/Router.hpp src/Hyperloglog.hpp src/MurmurHash3.hpp src/MurmurHash3.cpp)
target_link_libraries(TDHH ${Boost_LIBRARIES} ${qmax_LIBRARIES})

