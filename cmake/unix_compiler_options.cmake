function(untwine_target_compile_settings target)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
        #
        # VERSION_GREATER_EQUAL doesn't come until cmake 3.7
        #
        if (NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 7.0)
            target_compile_options(${target} PRIVATE
                -Wno-implicit-fallthrough
                -Wno-int-in-bool-context
                -Wno-dangling-else
                -Wno-noexcept-type
            )
        endif()
        set(UNTWINE_COMPILER_GCC 1)
    elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        set(UNTWINE_COMPILER_CLANG 1)
    else()
        message(FATAL_ERROR "Unsupported C++ compiler")
    endif()

    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -Wpointer-arith
        -Wcast-align
        -Wcast-qual
        -Wno-error=parentheses
        -Wno-error=cast-qual
        -Wredundant-decls

        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-long-long
        -Wno-unknown-pragmas
        -Wno-deprecated-declarations

        -Werror
    )
    if (UNTWINE_COMPILER_CLANG)
        target_compile_options(${target} PRIVATE
            -Wno-unknown-warning-option
        )
    endif()
endfunction()
