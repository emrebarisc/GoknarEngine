cmake_minimum_required(VERSION 3.20)

project(IMGUI)

set(CMAKE_CXX_STANDARD 17)

# 1. Collect Core ImGui source files (imgui.cpp, imgui_draw.cpp, etc.)
file(GLOB IMGUI_CORE_SOURCE 
    "Repo/*.cpp" 
    "Repo/*.c"
)

add_library(IMGUI STATIC ${IMGUI_CORE_SOURCE})

target_include_directories(IMGUI PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Repo>
)