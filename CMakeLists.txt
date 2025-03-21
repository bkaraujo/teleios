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
#                   TELIOS
#
# ==========================================
#add_definitions(-DTELEIOS_BUILD_RELEASE)

include_directories("engine/src/main")
add_executable(${PROJECT_NAME}
        engine/src/main/main.c
        engine/src/main/glad/glad.c

        engine/src/main/teleios/core/ulid.c
        engine/src/main/teleios/core/event.c
        engine/src/main/teleios/application.c
        engine/src/main/teleios/core/string.c
        engine/src/main/teleios/core/logger.c
        engine/src/main/teleios/core/graphics.c
        engine/src/main/teleios/core/platform.c
        engine/src/main/teleios/core/container_list.c
        engine/src/main/teleios/core/container_stack.c
        engine/src/main/teleios/core/platform_linux.c
        engine/src/main/teleios/core/platform_windows.c
        engine/src/main/teleios/core/profiler.c
        engine/src/main/teleios/core/meta.c

        engine/src/main/teleios/runtime/serializer.c
        engine/src/main/teleios/runtime/scene.h
)

target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} cglm)
target_link_libraries(${PROJECT_NAME} yaml)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "teleios")
