cmake_minimum_required(VERSION 3.5)
project(EPF VERSION 1.0 LANGUAGES CXX)

find_package(Pdal REQUIRED)

file(GLOB SRCS *.cpp)

add_executable(bu ${SRCS})

target_include_directories(bu PRIVATE ${PDAL_INCLUDE_DIRS})
target_link_libraries(bu PRIVATE ${PDAL_LIBRARIES})

set_property(TARGET bu PROPERTY CXX_STANDARD 11)
set_property(TARGET bu PROPERTY CXX_STANDARD_REQUIRED TRUE)
