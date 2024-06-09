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
	${SOURCE_DIR_NAME}/Goknar/Debug
	${SOURCE_DIR_NAME}/Goknar/Delegates
	${SOURCE_DIR_NAME}/Goknar/Editor
	${SOURCE_DIR_NAME}/Goknar/Editor/ImGuiEditor
	${SOURCE_DIR_NAME}/Goknar/Geometry
	${SOURCE_DIR_NAME}/Goknar/Helpers
	${SOURCE_DIR_NAME}/Goknar/IO
	${SOURCE_DIR_NAME}/Goknar/Lights
	${SOURCE_DIR_NAME}/Goknar/Lights/LightManager
	${SOURCE_DIR_NAME}/Goknar/Managers
	${SOURCE_DIR_NAME}/Goknar/Materials
	${SOURCE_DIR_NAME}/Goknar/Math
	${SOURCE_DIR_NAME}/Goknar/Model
	#${SOURCE_DIR_NAME}/Goknar/Model/2D
	${SOURCE_DIR_NAME}/Goknar/Physics
	${SOURCE_DIR_NAME}/Goknar/Physics/Components
	${SOURCE_DIR_NAME}/Goknar/Renderer
	${SOURCE_DIR_NAME}/Goknar/Renderer/MaterialEditor
	${SOURCE_DIR_NAME}/Goknar/UI
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
SET(ASSIMP_BUILD_FBX_IMPORTER TRUE CACHE BOOL "Enable FBX Importer")

##########################################################################################
#  We do not need these importers but there is a unresolved external symbol bug without  #
#                          Can be solved in the future by Assimp                         #
##########################################################################################
if(UNIX)
	SET(ASSIMP_BUILD_OBJ_IMPORTER TRUE CACHE BOOL "Enable OBJ Importer")
	SET(ASSIMP_BUILD_SMD_IMPORTER TRUE CACHE BOOL "Enable SMD Importer")
	SET(ASSIMP_BUILD_SIB_IMPORTER TRUE CACHE BOOL "Enable SIB Importer")
endif()
##########################################################################################

SET(ASSIMP_NO_EXPORT ON CACHE BOOL "Disable All Exporters")
add_subdirectory(thirdparty/assimp/)

SET(BUILD_BULLET2_DEMOS OFF CACHE BOOL "Disable Bullet3 BUILD_BULLET2_DEMOS")
SET(BUILD_BULLET_ROBOTICS_EXTRA OFF CACHE BOOL "Disable Bullet3 BUILD_BULLET_ROBOTICS_EXTRA")
SET(BUILD_BULLET_ROBOTICS_GUI_EXTRA OFF CACHE BOOL "Disable Bullet3 BUILD_BULLET_ROBOTICS_GUI_EXTRA")
SET(BUILD_CLSOCKET OFF CACHE BOOL "Disable Bullet3 BUILD_CLSOCKET")
SET(BUILD_CPU_DEMOS OFF CACHE BOOL "Disable Bullet3 BUILD_CPU_DEMOS")
SET(BUILD_ENET OFF CACHE BOOL "Disable Bullet3 BUILD_ENET")
SET(BUILD_EXTRAS OFF CACHE BOOL "Disable Bullet3 BUILD_EXTRAS")
SET(BUILD_GIMPACTUTILS_EXTRA OFF CACHE BOOL "Disable Bullet3 BUILD_GIMPACTUTILS_EXTRA")
SET(BUILD_HACD_EXTRA OFF CACHE BOOL "Disable Bullet3 BUILD_HACD_EXTRA")
SET(BUILD_INVERSE_DYNAMIC_EXTRA OFF CACHE BOOL "Disable Bullet3 BUILD_INVERSE_DYNAMIC_EXTRA")
SET(BUILD_OBJ2SDF_EXTRA OFF CACHE BOOL "Disable Bullet3 BUILD_OBJ2SDF_EXTRA")
SET(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "Disable Bullet3 BUILD_OPENGL3_DEMOS")
SET(BUILD_PYBULLET OFF CACHE BOOL "Disable Bullet3 BUILD_PYBULLET")
SET(BUILD_SERIALIZE_EXTRA OFF CACHE BOOL "Disable Bullet3 BUILD_SERIALIZE_EXTRA")
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable Bullet3 BUILD_SHARED_LIBS")
SET(BUILD_UNIT_TESTS OFF CACHE BOOL "Disable Bullet3 BUILD_UNIT_TESTS")
SET(BULLET2_MULTITHREADING OFF CACHE BOOL "Disable Bullet3 BULLET2_MULTITHREADING")
SET(USE_GLUT OFF CACHE BOOL "Disable Bullet3 USE_GLUT")
SET(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "Disable Bullet3 USE_GRAPHICAL_BENCHMARK")
SET(USE_OPENVR OFF CACHE BOOL "Disable Bullet3 USE_OPENVR")
SET(USE_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD OFF CACHE BOOL "Disable Bullet3 USE_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD")
add_subdirectory(thirdparty/bullet3/)


target_link_libraries(${APP_NAME} PUBLIC GLAD)
target_link_libraries(${APP_NAME} PUBLIC glfw)
target_link_libraries(${APP_NAME} PUBLIC IMGUI)
target_link_libraries(${APP_NAME} PUBLIC TINYXML)
target_link_libraries(${APP_NAME} PUBLIC spdlog)
target_link_libraries(${APP_NAME} PUBLIC assimp)
target_link_libraries(${APP_NAME} PUBLIC BulletDynamics BulletCollision LinearMath)

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
	thirdparty/bullet3/src
)


target_compile_definitions(${APP_NAME} PUBLIC "CONTENT_DIR=\"${CMAKE_BINARY_DIR}/Content/\"")

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
	file(COPY "EngineContent" DESTINATION ${CMAKE_BINARY_DIR}/)
	target_compile_definitions(${APP_NAME} PUBLIC "ENGINE_CONTENT_DIR=\"${CMAKE_BINARY_DIR}/EngineContent/\"")
	add_compile_definitions(GOKNAR_BUILD_DEBUG GOKNAR_ENABLE_ASSERTS)
	message("-------------------- Debug --------------------")
elseif(${CMAKE_BUILD_TYPE} STREQUAL Release)
	add_compile_definitions(GOKNAR_BUILD_RELEASE)
	message("-------------------- Release --------------------")
endif()

add_compile_definitions(GOKNAR_BUILD_DLL GLFW_INCLUDE_NONE)

target_precompile_headers(${APP_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:pch.h>")