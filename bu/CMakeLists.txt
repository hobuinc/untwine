cmake_minimum_required(VERSION 3.5)
project(EPF VERSION 1.0 LANGUAGES CXX)

find_package(PDAL REQUIRED)
if(PDAL_VERSION_MAJOR LESS 2)
  message(FATAL_ERROR "PDAL version is too old (${PDAL_VERSION}). Use 2.2 or higher.")
endif()
if((PDAL_VERSION_MAJOR EQUAL 2) AND (PDAL_VERSION_MINOR LESS 2))
  message(FATAL_ERROR "PDAL version is too old (${PDAL_VERSION}). Use 2.2 or higher.")
endif()

file(GLOB SRCS *.cpp)

add_executable(bu ${SRCS})

target_include_directories(bu PRIVATE ${PDAL_INCLUDE_DIRS})
target_link_libraries(bu PRIVATE ${PDAL_LIBRARIES} pthread)

set_property(TARGET bu PROPERTY CXX_STANDARD 11)
set_property(TARGET bu PROPERTY CXX_STANDARD_REQUIRED TRUE)
