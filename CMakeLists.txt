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
    find_library(DL_LIB dl)

    target_link_libraries(${PROJECT_NAME}
        ${ANDROID_LIB}
        ${MEDIA_NDK_LIB}
        ${LOG_LIB}
        ${EGL_LIB}
        ${GLES_LIB}
        ${DL_LIB}
    )

    if(NOT DEFINED ENV{FFMPEG_ROOT})
        message(FATAL_ERROR "FFMPEG_ROOT is not defined")
    endif()

    set(FFMPEG_ROOT "$ENV{FFMPEG_ROOT}")

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${FFMPEG_ROOT}/include"
    )

    target_link_libraries(${PROJECT_NAME}
        "${FFMPEG_ROOT}/lib/libavformat.a"
        "${FFMPEG_ROOT}/lib/libavcodec.a"
        "${FFMPEG_ROOT}/lib/libswscale.a"
        "${FFMPEG_ROOT}/lib/libswresample.a"
        "${FFMPEG_ROOT}/lib/libavutil.a"
        mediandk
        android
        log
        jnigraphics
        z
        m
        dl
    )

endif()
