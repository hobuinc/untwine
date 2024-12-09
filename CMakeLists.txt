cmake_minimum_required(VERSION 3.5)
project(Untwine VERSION 1.5.0 LANGUAGES CXX)
include(FeatureSummary)
include(${PROJECT_SOURCE_DIR}/cmake/compiler_options.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/cpack.cmake)

option(BUILD_TESTING "Chose if Untwine unit tests should be built" OFF)
add_feature_info("Unit tests" BUILD_TESTING "Untwine unit tests")

if(DEFINED WITH_TESTS)
  message(DEPRECATION "WITH_TESTS has been replaced with the standard CMake BUILD_TESTING variable")
  set(BUILD_TESTING ${WITH_TESTS})
endif()

find_package(PDAL REQUIRED CONFIG 2.6)

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
    prep/*.cpp
    epf/*.cpp
    bu/*.cpp
)

# MINGW must come before WIN32 test
# APPLE must come before LINUX test
if (MINGW)
    set(UNTWINE_OS_DIR untwine/mingw)
elseif (WIN32)
    set(UNTWINE_OS_DIR untwine/windows)
elseif (APPLE)
    set(UNTWINE_OS_DIR untwine/osx)
elseif (LINUX)
    set(UNTWINE_OS_DIR untwine/generic)
else()
    message(FATAL_ERROR "OS not supported")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_executable(untwine ${SRCS} ${LAZPERF_SRCS})

untwine_target_compile_settings(untwine)

target_include_directories(untwine
    PRIVATE
        ${UNTWINE_OS_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${PDAL_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(untwine
    PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
        ${PDAL_LIBRARIES}
)

if (MSVC)
    target_link_options(untwine
        PRIVATE
            /SUBSYSTEM:CONSOLE /ENTRY:wmainCRTStartup
    )
endif()

install(TARGETS untwine DESTINATION bin)

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

#
# CPACK
#
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)


