cmake_minimum_required(VERSION 3.20)

set(APP_NAME GameProject)

set(CMAKE_CXX_STANDARD 11)

project(${APP_NAME})

set(ENGINE_PATH ${CMAKE_BINARY_DIR}/../../Goknar)

set(all_code_files "src/Game.cpp")

if(WIN32 OR MSVC)
    add_compile_definitions(GOKNAR_PLATFORM_WINDOWS)
elseif(UNIX)
    add_compile_definitions(GOKNAR_PLATFORM_UNIX)
endif()

if(UNIX)
    SET(GCC_COVERAGE_LINK_FLAGS "-ldl -lglfw -lglut -lGLU -lGL")
    add_definitions(${GCC_COVERAGE_LINK_FLAGS})
endif()

if(NOT ANDROID)
    add_executable(${APP_NAME} ${all_code_files})
else()
    add_library(${APP_NAME} SHARED ${all_code_files})
    target_link_libraries(${APP_NAME} -Wl,--whole-archive cpp_android_spec -Wl,--no-whole-archive)
endif()

add_subdirectory(${ENGINE_PATH} ./Engine/Goknar)

target_link_libraries(${APP_NAME} PUBLIC GOKNAR)

target_include_directories(${APP_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/src)

add_compile_definitions(GOKNAR_BUILD_DLL GOKNAR_ENABLE_ASSERTS GLFW_INCLUDE_NONE)

file(COPY "Content" DESTINATION ${CMAKE_BINARY_DIR}/)





# target_precompile_headers(${APP_NAME} PRIVATE 
#    <fstream> 
#    <iostream>
#    <sstream>
#    <memory>
#    <limits>
#    <cmath>
#    <algorithm>
#    <functional>
#    <utility>
#    <stdexcept>
#    <chrono>
#    <string>
#    <vector>
#    <map>)