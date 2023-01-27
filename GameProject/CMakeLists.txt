cmake_minimum_required(VERSION 3.20)

set(APP_NAME GameProject)

set(CMAKE_CXX_STANDARD 11)

project(${APP_NAME})

set(SOURCE_DIR_NAME Source)

set(ENGINE_PATH ${CMAKE_BINARY_DIR}/../../Goknar)

if(WIN32 OR MSVC)
    add_compile_definitions(GOKNAR_PLATFORM_WINDOWS)
elseif(UNIX)
    add_compile_definitions(GOKNAR_PLATFORM_UNIX)
endif()

######################################################################
##############################	FILES	##############################
######################################################################

set(GAME_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
set(SUBDIRS
	${SOURCE_DIR_NAME}
)
set(GAME_SOURCE)

foreach(SUBDIR ${SUBDIRS})
	set(COMBINED_DIR ${GAME_SOURCE_DIR}${SUBDIR})
	file(GLOB GROUP_FILES "${COMBINED_DIR}/*" )
	source_group(${SUBDIR} FILES ${GROUP_FILES})
	list(APPEND GAME_SOURCE ${GROUP_FILES})
endforeach()

######################################################################

if(UNIX)
    SET(GCC_COVERAGE_LINK_FLAGS "-ldl -lglfw -lglut -lGLU -lGL")
    add_definitions(${GCC_COVERAGE_LINK_FLAGS})
endif()

if(NOT ANDROID)
    add_executable(${APP_NAME} ${GAME_SOURCE})
else()
    add_library(${APP_NAME} SHARED ${GAME_SOURCE})
    target_link_libraries(${APP_NAME} -Wl,--whole-archive cpp_android_spec -Wl,--no-whole-archive)
endif()

add_subdirectory(${ENGINE_PATH} ./Engine/Goknar)

target_link_libraries(${APP_NAME} PUBLIC GOKNAR)

target_include_directories(${APP_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/${SOURCE_DIR_NAME})

add_compile_definitions(GOKNAR_BUILD_DLL GOKNAR_ENABLE_ASSERTS GLFW_INCLUDE_NONE)

file(COPY "Content" DESTINATION ${CMAKE_BINARY_DIR}/)