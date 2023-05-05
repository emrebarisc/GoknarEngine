cmake_minimum_required(VERSION 3.20)

set(APP_NAME GOKNAR)

project(${APP_NAME})

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_DIR_NAME "Source")

######################################################################
##############################	FILES	##############################
######################################################################

set(GOKNAR_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
set(SUBDIRS
	${SOURCE_DIR_NAME}
	${SOURCE_DIR_NAME}/Goknar
	${SOURCE_DIR_NAME}/Goknar/Components
	#${SOURCE_DIR_NAME}/Goknar/Components/2D
	${SOURCE_DIR_NAME}/Goknar/Contents
	${SOURCE_DIR_NAME}/Goknar/Containers
	${SOURCE_DIR_NAME}/Goknar/Editor
	${SOURCE_DIR_NAME}/Goknar/Editor/ImGuiEditor
	${SOURCE_DIR_NAME}/Goknar/Geometry
	${SOURCE_DIR_NAME}/Goknar/Helpers
	${SOURCE_DIR_NAME}/Goknar/IO
	${SOURCE_DIR_NAME}/Goknar/Lights
	${SOURCE_DIR_NAME}/Goknar/Lights/ShadowManager
	${SOURCE_DIR_NAME}/Goknar/Managers
	${SOURCE_DIR_NAME}/Goknar/Math
	${SOURCE_DIR_NAME}/Goknar/Model
	#${SOURCE_DIR_NAME}/Goknar/Model/2D
	${SOURCE_DIR_NAME}/Goknar/Physics
	${SOURCE_DIR_NAME}/Goknar/Renderer
	${SOURCE_DIR_NAME}/Goknar/Renderer/MaterialEditor
)

set(ENGINE_SOURCE)
foreach(SUBDIR ${SUBDIRS})
	set(COMBINED_DIR ${GOKNAR_SOURCE_DIR}${SUBDIR})
	file(GLOB GROUP_FILES "${COMBINED_DIR}/*" )
	source_group(${SUBDIR} FILES ${GROUP_FILES})
	list(APPEND ENGINE_SOURCE ${GROUP_FILES})
endforeach()
######################################################################

if(WIN32 OR MSVC)
    add_compile_definitions(GOKNAR_PLATFORM_WINDOWS)
elseif(UNIX)
    add_compile_definitions(GOKNAR_PLATFORM_UNIX)
endif()

if(NOT ANDROID)
    add_library(${APP_NAME} ${ENGINE_SOURCE})
else()
    add_library(${APP_NAME} SHARED ${ENGINE_SOURCE})
    target_link_libraries(${APP_NAME} -Wl,--whole-archive cpp_android_spec -Wl,--no-whole-archive)
endif()

add_subdirectory(thirdparty/GLAD/)
add_subdirectory(thirdparty/glfw/)
add_subdirectory(thirdparty/imgui/)
add_subdirectory(thirdparty/spdlog/)
add_subdirectory(thirdparty/TinyXML/)

# Assimp
SET(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT FALSE CACHE BOOL "Disable All Importers")
SET(ASSIMP_BUILD_FBX_IMPORTER TRUE CACHE BOOL "Enable Fbx Importer")
SET(ASSIMP_NO_EXPORT ON CACHE BOOL "Disable All Exporters")
add_subdirectory(thirdparty/assimp/)

target_link_libraries(${APP_NAME} PUBLIC GLAD)
target_link_libraries(${APP_NAME} PUBLIC glfw)
target_link_libraries(${APP_NAME} PUBLIC IMGUI)
target_link_libraries(${APP_NAME} PUBLIC TINYXML)
target_link_libraries(${APP_NAME} PUBLIC spdlog)
target_link_libraries(${APP_NAME} PUBLIC assimp)

if(UNIX)
    SET(GCC_COVERAGE_LINK_FLAGS "-ldl -lglfw -lglut -lGLU -lGL")
    add_definitions(${GCC_COVERAGE_LINK_FLAGS})
endif()

target_include_directories(${APP_NAME} PUBLIC 
	${SOURCE_DIR_NAME}
	${SOURCE_DIR_NAME}/Goknar
	thirdparty
	thirdparty/GLAD/GLAD/include
	thirdparty/GLFW/include
	thirdparty/imgui/imgui
	thirdparty/spdlog/include
	thirdparty/TinyXML/include
	thirdparty/assimp/include
)

target_compile_definitions(${APP_NAME} PUBLIC "CONTENT_DIR=\"${CMAKE_BINARY_DIR}/Content/\"")

add_compile_definitions(GOKNAR_BUILD_DLL GOKNAR_ENABLE_ASSERTS GLFW_INCLUDE_NONE)

target_precompile_headers(${APP_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:pch.h>")