cmake_minimum_required(VERSION 3.20)

project(sandbox LANGUAGES C)

# Collect sandbox source files
set(SANDBOX_SOURCES
    src/main/main.c
)

# Create sandbox executable
add_library(sandbox SHARED
    ${SANDBOX_SOURCES}
)

# Set target properties
set_target_properties(sandbox PROPERTIES
    OUTPUT_NAME "sandbox"
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

# Include engine headers (for reference only, sandbox is standalone)
target_include_directories(sandbox
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/src/main
)

# Link GLFW and cglm directly
target_link_libraries(sandbox
    PRIVATE
        glfw
        cglm
)

# Preprocessor definitions
target_compile_definitions(sandbox
    PRIVATE
        $<$<CONFIG:Debug>:TELEIOS_BUILD_DEBUG>
        $<$<CONFIG:Release>:TELEIOS_BUILD_RELEASE>
)

# Compiler flags
if(MSVC)
    target_compile_options(sandbox PRIVATE
        /W4                      # Warning level 4
        /std:c11                 # C11 standard
        /experimental:c11atomics # Enable C11 atomics support
        $<$<CONFIG:Debug>:/Od>   # No optimization in debug
        $<$<CONFIG:Debug>:/Zi>   # Debug symbols
        $<$<CONFIG:Release>:/O2> # Optimize for speed
    )
    target_link_options(sandbox PRIVATE
        $<$<CONFIG:Debug>:/DEBUG>
        /SUBSYSTEM:CONSOLE
    )
else()
    target_compile_options(sandbox PRIVATE
        -Wall
        -Wextra
        -std=c11
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O3>
    )
    # Add -gcodeview only for Clang (not supported by GCC)
    if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
        target_compile_options(sandbox PRIVATE
            $<$<CONFIG:Debug>:-gcodeview>
        )
    endif()
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(sandbox
        PRIVATE
            user32
            gdi32
            kernel32
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(sandbox
        PRIVATE
            m
            pthread
    )
endif()

# Post-build: Copy sandbox dll to dist directory
add_custom_command(TARGET sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/dist
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sandbox> ${CMAKE_SOURCE_DIR}/dist/
    COMMENT "Copying sandbox.dll to dist directory"
)

