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
    target_compile_definitions(yaml PRIVATE
        YAML_DECLARE_STATIC
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

# Download and configure cglm (OpenGL Mathematics library) using modern file() approach
set(CGLM_URL "https://github.com/recp/cglm/archive/refs/tags/v0.9.4.tar.gz")
set(CGLM_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/_deps/cglm-download")
set(CGLM_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/cglm-src")

# Download and extract if not already done
if(NOT EXISTS "${CGLM_SOURCE_DIR}/include")
    message(STATUS "Downloading cglm...")
    file(DOWNLOAD "${CGLM_URL}" "${CGLM_DOWNLOAD_DIR}/cglm.tar.gz"
         SHOW_PROGRESS
         STATUS CGLM_DOWNLOAD_STATUS)

    list(GET CGLM_DOWNLOAD_STATUS 0 CGLM_DOWNLOAD_CODE)
    if(NOT CGLM_DOWNLOAD_CODE EQUAL 0)
        message(FATAL_ERROR "Failed to download cglm")
    endif()

    message(STATUS "Extracting cglm...")
    file(ARCHIVE_EXTRACT
         INPUT "${CGLM_DOWNLOAD_DIR}/cglm.tar.gz"
         DESTINATION "${CMAKE_BINARY_DIR}/_deps")

    # Rename extracted directory
    file(RENAME "${CMAKE_BINARY_DIR}/_deps/cglm-0.9.4" "${CGLM_SOURCE_DIR}")
endif()

# Build cglm as a static library
file(GLOB CGLM_SOURCES "${CGLM_SOURCE_DIR}/src/*.c")

add_library(cglm STATIC ${CGLM_SOURCES})
target_include_directories(cglm PUBLIC
    "${CGLM_SOURCE_DIR}/include"
)
set_target_properties(cglm PROPERTIES
    OUTPUT_NAME "cglm"
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Define CGLM_STATIC for static linking
target_compile_definitions(cglm PUBLIC CGLM_STATIC)

# Collect engine source files
set(ENGINE_SOURCES
    src/main/main.c
    src/main/teleios/logger.c
    src/main/teleios/memory.c
    src/main/teleios/platform.c
    src/main/teleios/profiler.c
    src/main/teleios/application.c
)

# Collect engine header files
set(ENGINE_HEADERS
    src/main/teleios/event.h
    src/main/teleios/logger.h
    src/main/teleios/memory.h
    src/main/teleios/window.h
    src/main/teleios/defines.h
    src/main/teleios/teleios.h
    src/main/teleios/platform.h
    src/main/teleios/profiler.h
    src/main/teleios/application.h
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
        $<$<CONFIG:Debug>:-gcodeview>
        $<$<CONFIG:Release>:-O3>
    )
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

