
set(GOOGLETEST_VERSION 1.12.1)

# Since tests link CRT dynamically (/MD[d]), require gtest to
#link dynamically too (default is /MT[d])
option(gtest_force_shared_crt "Always use shared Visual C++ run-time DLL" ON)

add_subdirectory(gtest)

###############################################################################
# Add a test target.
# _name The driver name.
# ARGN :
#    FILES the source files for the test
#    LINK_WITH link test executable with libraries
#    INCLUDES header file directories
#
macro(UNTWINE_ADD_TEST _name)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs FILES LINK_WITH INCLUDES SYSTEM_INCLUDES)
    cmake_parse_arguments(UNTWINE_ADD_TEST
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    add_executable(${_name}
        ${UNTWINE_ADD_TEST_FILES}
        ${LAZPERF_SRCS}
    )
    untwine_target_compile_settings(${_name})
    target_include_directories(${_name} PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PDAL_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
        gtest/include
    )
    set_property(TARGET ${_name} PROPERTY FOLDER "Tests")
    target_link_libraries(${_name}
        PRIVATE
            gtest
            ${PDAL_LIBRARIES}
            ${UNTWINE_ADD_TEST_LINK_WITH}
    )
    add_test(NAME ${_name}
        COMMAND
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_name}"
        WORKING_DIRECTORY
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/.."
    )
endmacro()

configure_file(TestConfig.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/TestConfig.hpp)

UNTWINE_ADD_TEST(untwine_test FILES Tests.cpp)
