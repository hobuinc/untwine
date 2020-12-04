cmake_minimum_required(VERSION 3.5)
project(Untwine VERSION 1.0 LANGUAGES CXX)
include(FeatureSummary)
include(${PROJECT_SOURCE_DIR}/cmake/compiler_options.cmake)

find_package(PDAL REQUIRED)
if (PDAL_VERSION_MAJOR LESS 2)
  message(FATAL_ERROR "PDAL version is too old (${PDAL_VERSION}). Use 2.2 or higher.")
endif()
if ((PDAL_VERSION_MAJOR EQUAL 2) AND (PDAL_VERSION_MINOR LESS 2))
  message(FATAL_ERROR "PDAL version is too old (${PDAL_VERSION}). Use 2.2 or higher.")
endif()

find_package(Threads)
set_package_properties(Threads PROPERTIES DESCRIPTION
    "The thread library of the system" TYPE REQUIRED)

file(GLOB SRCS
    untwine/*.cpp
    epf/*.cpp
    bu/*.cpp
)

add_executable(untwine ${SRCS})

untwine_target_compile_settings(untwine)

target_include_directories(untwine
    PRIVATE
        ${PDAL_INCLUDE_DIRS}
)
target_link_libraries(untwine
    PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
        ${PDAL_LIBRARIES}
)

install(TARGETS untwine DESTINATION bin)