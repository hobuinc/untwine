cmake_minimum_required(VERSION 3.5)
project(Untwine VERSION 1.1.0 LANGUAGES CXX)
include(FeatureSummary)
include(${PROJECT_SOURCE_DIR}/cmake/compiler_options.cmake)

find_package(PDAL REQUIRED)
if ((PDAL_VERSION_MAJOR EQUAL 1) AND (PDAL_VERSION_MINOR LESS 7))
  message(FATAL_ERROR "PDAL version is too old (${PDAL_VERSION}). Use 1.7 or higher.")
endif()

find_package(Threads)
set_package_properties(Threads PROPERTIES DESCRIPTION
    "The thread library of the system" TYPE REQUIRED)

configure_file(untwine/Config.hpp.in include/Config.hpp)

file(GLOB LAZPERF_SRCS
    lazperf/*.cpp
    lazperf/detail/*.cpp
)

file(GLOB SRCS
    untwine/*.cpp
    epf/*.cpp
    bu/*.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_executable(untwine ${SRCS} ${LAZPERF_SRCS})

untwine_target_compile_settings(untwine)

target_include_directories(untwine
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${PDAL_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}
)
target_link_libraries(untwine
    PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
        ${PDAL_LIBRARIES}
)
if(MSVC)
    target_link_options(untwine
        PRIVATE
            /SUBSYSTEM:WINDOWS /ENTRY:wmainCRTStartup"
    )
endif()

install(TARGETS untwine DESTINATION bin)

#-----------------

enable_testing()
add_subdirectory(test)

