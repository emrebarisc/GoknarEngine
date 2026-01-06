cmake_minimum_required(VERSION 3.20)

set(APP_NAME GoknarEngineTests)

project(${APP_NAME})

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_DIR_NAME "Source")

######################################################################
##############################	FILES	##############################
######################################################################

set(TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
set(SUBDIRS
    ${SOURCE_DIR_NAME}
    ${SOURCE_DIR_NAME}/Delegates
    ${SOURCE_DIR_NAME}/Math
)
set(TEST_SOURCE)

foreach(SUBDIR ${SUBDIRS})
	set(COMBINED_DIR ${TEST_SOURCE_DIR}${SUBDIR})
	file(GLOB GROUP_FILES "${COMBINED_DIR}/*" )
	source_group(${SUBDIR} FILES ${GROUP_FILES})
	list(APPEND TEST_SOURCE ${GROUP_FILES})
endforeach()

######################################################################

set(ENGINE_PATH ${CMAKE_BINARY_DIR}/../../Goknar/)

if(WIN32 OR MSVC)
    add_compile_definitions(GOKNAR_PLATFORM_WINDOWS)
elseif(UNIX)
    add_compile_definitions(GOKNAR_PLATFORM_UNIX)
endif()

if(UNIX)
    SET(GCC_COVERAGE_LINK_FLAGS "-ldl -lglfw -lglut -lGLU -lGL")
    add_definitions(${GCC_COVERAGE_LINK_FLAGS})
endif()

add_executable(${APP_NAME} ${TEST_SOURCE})

add_subdirectory(${ENGINE_PATH} ./Goknar)

target_link_libraries(${APP_NAME} PUBLIC GOKNAR)

target_include_directories(${APP_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/${SOURCE_DIR_NAME})

add_compile_definitions(GOKNAR_BUILD_DLL GOKNAR_ENABLE_ASSERTS GLFW_INCLUDE_NONE NOMINMAX)