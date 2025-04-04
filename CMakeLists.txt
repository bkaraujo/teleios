cmake_minimum_required(VERSION 3.30)
set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE debug)

project(teleios C)

# ==========================================
#
#                   GLFW
#
# ==========================================
add_subdirectory(engine/libs/glfw)
include_directories(engine/libs/glfw/include)
set(GLFW_LIBRARY_TYPE STATIC)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# ==========================================
#
#                   CGLM
#
# ==========================================
set(CGLM_USE_C99 ON CACHE BOOL "" FORCE)
set(CGLM_STATIC ON CACHE BOOL "Static option" FORCE)
set(CGLM_SHARED OFF CACHE BOOL "Shared option" FORCE)
set(CGLM_USE_TEST ON CACHE BOOL "Enable Tests" FORCE)
add_subdirectory(engine/libs/cglm)
include_directories(engine/libs/cglm/include)
# ==========================================
#
#                   LIBYAML
#
# ==========================================
add_subdirectory(engine/libs/libyaml)
include_directories(engine/libs/libyaml/include)
# ==========================================
#
#                   LUA
#
# ==========================================
add_subdirectory(engine/libs/liblua)

# ==========================================
#
#                   TELIOS
#
# ==========================================
#add_definitions(-DTELEIOS_BUILD_RELEASE)

include_directories("engine/src/main")
add_executable(${PROJECT_NAME}
        engine/src/main/main.c
        engine/src/main/glad/glad.c

        engine/src/main/teleios/core.c
        engine/src/main/teleios/runtime.c
        engine/src/main/teleios/system.c
        engine/src/main/teleios/application.c
        engine/src/main/teleios/container.c
        engine/src/main/teleios/string.c
        engine/src/main/teleios/event.c
        engine/src/main/teleios/number.c
        engine/src/main/teleios/platform.c
        engine/src/main/teleios/graphics.c
        engine/src/main/teleios/scene.c
)

target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-unused-parameter")

if (MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /W4 /permissive)
else ()
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra)
endif()

target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} cglm)
target_link_libraries(${PROJECT_NAME} yaml)
target_link_libraries(${PROJECT_NAME} lua_static)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "teleios")
