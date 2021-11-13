cmake_minimum_required(VERSION 3.20)

set(APP_NAME GOKNAR)

project(${APP_NAME})

set(CMAKE_CXX_STANDARD 11)

set(GAME_SOURCE)
set(GAME_HEADER)

file (GLOB_RECURSE sourceFiles "src/*.c" "src/*.cpp" "src/*.cxx")
file (GLOB_RECURSE headerFiles "src/*.h" "src/*.hpp" "src/*.hxx")

list(APPEND GAME_SOURCE ${sourceFiles})
list(APPEND GAME_HEADER ${headerFiles})

set(all_code_files ${GAME_HEADER} ${GAME_SOURCE} )

if(WIN32 OR MSVC)
    add_compile_definitions(GOKNAR_PLATFORM_WINDOWS)
elseif(UNIX)
    add_compile_definitions(GOKNAR_PLATFORM_UNIX)
endif()


if(NOT ANDROID)
    add_library(${APP_NAME} ${GAME_SOURCE})
else()
    add_library(${APP_NAME} SHARED ${all_code_files})
    target_link_libraries(${APP_NAME} -Wl,--whole-archive cpp_android_spec -Wl,--no-whole-archive)
endif()


add_subdirectory(thirdparty/GLAD/)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/imgui/)
add_subdirectory(thirdparty/spdlog/)
add_subdirectory(thirdparty/TinyXML/)

target_link_libraries(${APP_NAME} PUBLIC GLAD)
target_link_libraries(${APP_NAME} PUBLIC glfw)
target_link_libraries(${APP_NAME} PUBLIC IMGUI)
target_link_libraries(${APP_NAME} PUBLIC TINYXML)
target_link_libraries(${APP_NAME} PUBLIC spdlog)

if(UNIX)
    SET(GCC_COVERAGE_LINK_FLAGS "-ldl -lglfw -lglut -lGLU -lGL")
    add_definitions(${GCC_COVERAGE_LINK_FLAGS})
endif()

target_include_directories(${APP_NAME} PUBLIC 
	src
	src/Goknar
	thirdparty
	thirdparty/GLAD/GLAD/include
	thirdparty/GLFW/include
	thirdparty/imgui/imgui
	thirdparty/spdlog/include
	thirdparty/TinyXML/include)

target_compile_definitions(${APP_NAME} PUBLIC "CONTENT_DIR=\"${CMAKE_BINARY_DIR}/Content/\"")

add_compile_definitions(GOKNAR_BUILD_DLL GOKNAR_ENABLE_ASSERTS GLFW_INCLUDE_NONE)

target_precompile_headers(${APP_NAME} PRIVATE 
    <fstream> 
    <iostream>
    <sstream>
    <memory>
    <limits>
    <cmath>
    <algorithm>
    <functional>
    <utility>
    <stdexcept>
    <chrono>
    <string>
    <vector>
    <map>)