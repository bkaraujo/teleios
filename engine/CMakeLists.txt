cmake_minimum_required(VERSION 3.20)

project(engine LANGUAGES C)

# Include FetchContent module for downloading dependencies
include(FetchContent)

# Download and configure GLFW (latest version)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4  # Latest stable version
    GIT_SHALLOW    TRUE # Only download the latest commit
)

# GLFW configuration options
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

# Make GLFW available
FetchContent_MakeAvailable(glfw)

# Download and configure libyaml
FetchContent_Declare(
    libyaml
    GIT_REPOSITORY https://github.com/yaml/libyaml.git
    GIT_TAG        0.2.5  # Latest stable version
    GIT_SHALLOW    TRUE
)

# Download libyaml source
FetchContent_GetProperties(libyaml)
if(NOT libyaml_POPULATED)
    FetchContent_Populate(libyaml)

    # Build libyaml as a static library manually
    file(GLOB LIBYAML_SOURCES
        "${libyaml_SOURCE_DIR}/src/*.c"
    )

    add_library(yaml STATIC ${LIBYAML_SOURCES})
    target_include_directories(yaml PUBLIC
        "${libyaml_SOURCE_DIR}/include"
        "${libyaml_SOURCE_DIR}/src"
    )
    set_target_properties(yaml PROPERTIES
        OUTPUT_NAME "yaml"
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
    )

    # Platform-specific definitions for libyaml
    target_compile_definitions(yaml PUBLIC
        YAML_DECLARE_STATIC
    )
    target_compile_definitions(yaml PRIVATE
        YAML_VERSION_MAJOR=0
        YAML_VERSION_MINOR=2
        YAML_VERSION_PATCH=5
        YAML_VERSION_STRING="0.2.5"
    )
endif()

# Download and configure Lua
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG        v5.4.7  # Latest stable version
    GIT_SHALLOW    TRUE
)

# Download Lua source
FetchContent_GetProperties(lua)
if(NOT lua_POPULATED)
    FetchContent_Populate(lua)

    # Build Lua as a static library
    file(GLOB LUA_SOURCES
        "${lua_SOURCE_DIR}/*.c"
    )
    # Remove lua.c, luac.c and onelua.c (standalone executables and amalgamation)
    list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/lua\\.c$")
    list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/luac\\.c$")
    list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/onelua\\.c$")

    add_library(lua_static STATIC ${LUA_SOURCES})
    target_include_directories(lua_static PUBLIC ${lua_SOURCE_DIR})
    set_target_properties(lua_static PROPERTIES
        OUTPUT_NAME "lua"
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
    )

    # Platform-specific definitions for Lua
    if(UNIX AND NOT APPLE)
        target_compile_definitions(lua_static PUBLIC LUA_USE_LINUX)
    elseif(APPLE)
        target_compile_definitions(lua_static PUBLIC LUA_USE_MACOSX)
    elseif(WIN32)
        target_compile_definitions(lua_static PUBLIC LUA_USE_WINDOWS)
    endif()
endif()

# Download and configure cglm (OpenGL Mathematics library)
FetchContent_Declare(
    cglm
    GIT_REPOSITORY https://github.com/recp/cglm.git
    GIT_TAG        v0.9.4  # Latest stable version
    GIT_SHALLOW    TRUE
)

# cglm configuration options
set(CGLM_SHARED     OFF CACHE BOOL "" FORCE)
set(CGLM_STATIC     ON  CACHE BOOL "" FORCE)
set(CGLM_USE_C99    OFF CACHE BOOL "" FORCE)
set(CGLM_USE_TEST   OFF CACHE BOOL "" FORCE)

# Make cglm available
FetchContent_MakeAvailable(cglm)

# Collect engine source files
set(ENGINE_SOURCES
    src/main/glad/glad.c

    src/main/main.c
    src/main/teleios/logger.c
    src/main/teleios/memory.c
    src/main/teleios/platform.c
    src/main/teleios/application.c
    src/main/teleios/thread.c
    src/main/teleios/event.c
    src/main/teleios/number.c
    src/main/teleios/container.c
    src/main/teleios/strings.c
    src/main/teleios/config.c
    src/main/teleios/input.c
    src/main/teleios/profiler.c
)

# Collect engine header files
set(ENGINE_HEADERS
    include/glad/glad.h
    include/glad/khrplatform.h
    include/stb/image.h

    include/teleios/application.h
    include/teleios/chrono.h
    include/teleios/config.h
    include/teleios/container.h
    include/teleios/defines.h
    include/teleios/event.h
    include/teleios/filesystem.h
    include/teleios/input.h
    include/teleios/logger.h
    include/teleios/memory.h
    include/teleios/number.h
    include/teleios/platform.h
    include/teleios/profiler.h
    include/teleios/strings.h
    include/teleios/teleios.h
    include/teleios/thread.h
    include/teleios/window.h
)

# Create engine executable
add_executable(engine
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

# Set target properties
set_target_properties(engine PROPERTIES
    OUTPUT_NAME         "engine"
    C_STANDARD          11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS        OFF
)

# Include directories
target_include_directories(engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/main>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main/teleios
)

# Link libraries (GLFW, libyaml, Lua, cglm)
target_link_libraries(engine
    PUBLIC
        glfw
        yaml
        lua_static
        cglm
)

# Preprocessor definitions
target_compile_definitions(engine
    PRIVATE
        $<$<CONFIG:Debug>:TELEIOS_BUILD_DEBUG>
        $<$<CONFIG:Release>:TELEIOS_BUILD_RELEASE>
)

# Compiler flags
if(MSVC)
    target_compile_options(engine PRIVATE
        /W4                      # Warning level 4
        /std:c11                 # C11 standard
        /experimental:c11atomics # Enable C11 atomics support
        $<$<CONFIG:Debug>:/Od>   # No optimization in debug
        $<$<CONFIG:Debug>:/Zi>   # Debug symbols
        $<$<CONFIG:Release>:/O2> # Optimize for speed
    )
    target_link_options(engine PRIVATE
        $<$<CONFIG:Debug>:/DEBUG>
        /SUBSYSTEM:CONSOLE
    )
else()
    target_compile_options(engine PRIVATE
        -Wall
        -Wextra
        -std=c11
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O3>
    )
    # Add -gcodeview only for Clang (not supported by GCC)
    if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
        target_compile_options(engine PRIVATE
            $<$<CONFIG:Debug>:-gcodeview>
        )
    endif()
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(engine
        PRIVATE
            user32
            gdi32
            kernel32
            opengl32
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(engine
        PRIVATE
            m
            pthread
            dl
    )
    find_package(OpenGL REQUIRED)
    target_link_libraries(engine PRIVATE OpenGL::GL)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(engine
        PRIVATE
            ${COCOA_LIBRARY}
            ${IOKIT_LIBRARY}
            ${COREVIDEO_LIBRARY}
            OpenGL::GL
    )
endif()

# Post-build: Copy engine executable to dist directory
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/dist
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:engine> ${CMAKE_SOURCE_DIR}/dist/
    COMMENT "Copying engine.exe to dist directory"
)

