cmake_minimum_required(VERSION 3.30)
set(CMAKE_C_STANDARD 11)
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
set(CGLM_STATIC ON CACHE BOOL "Static option" FORCE)
set(CGLM_SHARED OFF CACHE BOOL "Shared option" FORCE)
add_subdirectory(engine/libs/cglm)
include_directories(engine/libs/cglm/include)
# ==========================================
#
#                   TELIOS
#
# ==========================================
include_directories("engine/src")
add_executable(${PROJECT_NAME}
    engine/src/main.c
    engine/src/stb/stb_image.h
    engine/src/teleios/application.c
    engine/src/teleios/container_list.c
    engine/src/teleios/logger.c
    engine/src/teleios/platform_linux.c
)

target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} cglm)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "teleios")
