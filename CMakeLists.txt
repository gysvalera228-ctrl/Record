cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(Record VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK!")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})

if(ANDROID)
    find_library(ANDROID_LIB android)
    find_library(MEDIA_NDK_LIB mediandk)
    find_library(LOG_LIB log)
    find_library(EGL_LIB EGL)
    find_library(GLES_LIB GLESv2)

    target_link_libraries(${PROJECT_NAME}
        ${ANDROID_LIB}
        ${MEDIA_NDK_LIB}
        ${LOG_LIB}
        ${EGL_LIB}
        ${GLES_LIB}
    )
endif()
