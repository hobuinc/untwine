cmake_minimum_required(VERSION 3.5)
project(Untwine VERSION 1.0 LANGUAGES CXX)
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

file(GLOB SRCS
    untwine/*.cpp
    epf/*.cpp
    bu/*.cpp
    lazperf/*.cpp
    lazperf/detail/*.cpp
)


if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        /SUBSYSTEM:WINDOWS /ENTRY:wmainCRTStartup")
endif()

add_executable(untwine ${SRCS})

if(MSVC)
    set_target_properties(untwine PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

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

install(TARGETS untwine DESTINATION bin)
