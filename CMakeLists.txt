cmake_minimum_required(VERSION 3.30)
set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE debug)

project(teleios C)
# ==========================================
#
#                   GLFW
#
# ==========================================
add_subdirectory(libs/glfw)
include_directories(libs/glfw/include)
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
add_subdirectory(libs/cglm)
include_directories(libs/cglm/include)
# ==========================================
#
#                  YAML
#
# ==========================================
add_subdirectory(libs/yaml)
include_directories(libs/yaml/include)
# ==========================================
#
#                   LUA
#
# ==========================================
add_subdirectory(libs/lua)
# ==========================================
#
#                   TELIOS
#
# ==========================================
include_directories(src/main)

add_executable(teleios
        src/main/glad/glad.c

        src/main/libk/ktime.c
        src/main/libk/klogger.c
        src/main/libk/kmemory.c
        src/main/libk/kruntime.c
        src/main/libk/kallocator.c
        src/main/libk/kstring.c
        src/main/libk/knumber.c
        src/main/libk/kcollection.c
        src/main/libk/kcontainer_list.c
        src/main/libk/kcontainer_stack.c
        src/main/libk/kcontainer_map.c
        src/main/libk/kcontainer_iterator.c

        src/main/main.c
        src/main/teleios/yaml.c
        src/main/teleios/filesystem.c
        src/main/teleios/runtime.c
        src/main/teleios/application.c
)

#set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "teleios")
#target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-unused-parameter")
#target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} cglm)
target_link_libraries(${PROJECT_NAME} yaml)
target_link_libraries(${PROJECT_NAME} lua_static)

