cmake_minimum_required(VERSION 3.20)

set(APP_NAME GOKNAR)

project(${APP_NAME})

set(CMAKE_CXX_STANDARD 17)

set(GAME_SOURCE)

file(GLOB SRC_FILES
	src/*
)
source_group(Source FILES ${SRC_FILES})
list(APPEND GAME_SOURCE ${SRC_FILES})

file(GLOB GOKNAR_FILES
	src/Goknar/*
)
source_group(Source/Goknar FILES ${GOKNAR_FILES})
list(APPEND GAME_SOURCE ${GOKNAR_FILES})

file(GLOB COMPONENTS_FILES
	src/Goknar/Components/*
)
source_group(Source/Goknar/Components FILES ${COMPONENTS_FILES})
list(APPEND GAME_SOURCE ${COMPONENTS_FILES})

file(GLOB CONTAINERS_FILES
	src/Goknar/Containers/*
)
source_group(Source/Goknar/Containers FILES ${CONTAINERS_FILES})
list(APPEND GAME_SOURCE ${CONTAINERS_FILES})

file(GLOB EDITOR_FILES
	src/Goknar/Editor/*
)
source_group(Source/Goknar/Editor FILES ${EDITOR_FILES})
list(APPEND GAME_SOURCE ${EDITOR_FILES})

file(GLOB IMGUI_EDITOR_FILES
	src/Goknar/Editor/ImGuiEditor/*
)
source_group(Source/Goknar/Editor/ImGuiEditor FILES ${IMGUI_EDITOR_FILES})
list(APPEND GAME_SOURCE ${IMGUI_EDITOR_FILES})

file(GLOB GEOMETRY_FILES
	src/Goknar/Geometry/*
)
source_group(Source/Goknar/Geometry FILES ${GEOMETRY_FILES})
list(APPEND GAME_SOURCE ${GEOMETRY_FILES})

file(GLOB HELPERS_FILES
	src/Goknar/Helpers/*
)
source_group(Source/Goknar/Helpers FILES ${HELPERS_FILES})
list(APPEND GAME_SOURCE ${HELPERS_FILES})

file(GLOB IO_FILES
	src/Goknar/IO/*
)
source_group(Source/Goknar/IO FILES ${IO_FILES})
list(APPEND GAME_SOURCE ${IO_FILES})

file(GLOB LIGHTS_FILES
	src/Goknar/Lights/*
)
source_group(Source/Goknar/Lights FILES ${LIGHTS_FILES})
list(APPEND GAME_SOURCE ${LIGHTS_FILES})

file(GLOB LIGHTS_SHADOW_MANAGER_FILES
	src/Goknar/Lights/ShadowManager/*
)
source_group(Source/Goknar/Lights/ShadowManager FILES ${LIGHTS_SHADOW_MANAGER_FILES})
list(APPEND GAME_SOURCE ${LIGHTS_SHADOW_MANAGER_FILES})

file(GLOB MANAGERS_FILES
	src/Goknar/Managers/*
)
source_group(Source/Goknar/Managers FILES ${MANAGERS_FILES})
list(APPEND GAME_SOURCE ${MANAGERS_FILES})

file(GLOB MODEL_FILES
	src/Goknar/Model/*
)
source_group(Source/Goknar/Model FILES ${MODEL_FILES})
list(APPEND GAME_SOURCE ${MODEL_FILES})

file(GLOB MODEL_2D_FILES
	src/Goknar/Model/2D/*
)
source_group(Source/Goknar/Model/2D FILES ${MODEL_2D_FILES})
list(APPEND GAME_SOURCE ${MODEL_2D_FILES})

file(GLOB RENDERER_FILES
	src/Goknar/Renderer/*
)
source_group(Source/Goknar/Renderer FILES ${RENDERER_FILES})
list(APPEND GAME_SOURCE ${RENDERER_FILES})

file(GLOB RENDERER_MATERIAL_EDITOR_FILES
	src/Goknar/Renderer/MaterialEditor/*
)
source_group(Source/Goknar/Renderer/MaterialEditor FILES ${RENDERER_MATERIAL_EDITOR_FILES})
list(APPEND GAME_SOURCE ${RENDERER_MATERIAL_EDITOR_FILES})

set(all_code_files ${GAME_SOURCE})

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

#target_precompile_headers(${APP_NAME} PRIVATE 
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

target_precompile_headers(${APP_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>")