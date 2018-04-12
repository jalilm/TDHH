cmake_minimum_required(VERSION 3.9)
project(TDHH)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost 1.58.0 COMPONENTS system filesystem REQUIRED serialization)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

include_directories(${Boost_INCLUDE_DIRS})
add_executable(TDHH src/main.cpp src/PacketsReader.cpp src/PacketsReader.hpp src/IPPacket.hpp src/CSVIterator.hpp src/Router.hpp src/QMax.hpp src/Hyperloglog.hpp src/MurmurHash3.hpp src/MurmurHash3.cpp)
target_link_libraries(TDHH ${Boost_LIBRARIES})

