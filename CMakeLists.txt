cmake_minimum_required(VERSION 3.9.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -g -fpermissive")

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)

project(GSTTestApp)

include_directories(
  ${GLIB_INCLUDE_DIRS}
)

link_directories(
  ${GLIB_LIBRARY_DIRS}
)


set(GSTREAMER_MINIMUM_VERSION 1.0.5)

pkg_check_modules(GST1_TEST gstreamer-1.0)
if ( GST1_TEST_FOUND AND NOT ${GST1_TEST_VERSION} VERSION_LESS ${GSTREAMER_MINIMUM_VERSION} )
    pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
    pkg_check_modules(GSTREAMER-APP REQUIRED gstreamer-app-1.0)
    pkg_check_modules(GSTREAMER-AUDIO REQUIRED gstreamer-audio-1.0)
    pkg_check_modules(GSTREAMER-PBUTILS REQUIRED gstreamer-pbutils-1.0)
    pkg_check_modules(GSTREAMER-FFT REQUIRED gstreamer-fft-1.0)
    add_definitions(-DGST_API_VERSION_1=1)
endif()

set(DRIVER_DIR_SRC                 src)
set(DRIVER_DIR_INC                 include)
set(DRIVER_TEMPLATE_TARGET_NAME             "GSTTestApp")

include_directories(
  ${GSTREAMER_INCLUDE_DIRS}
  ${GSTREAMER-APP_INCLUDE_DIRS}
  ${GSTREAMER-AUDIO_INCLUDE_DIRS}
  ${GSTREAMER-PBUTILS_INCLUDE_DIRS}
  ${GSTREAMER-FFT_INCLUDE_DIRS}
  ${DRIVER_DIR_INC}
)

link_directories(
  ${GSTREAMER_LIBRARY_DIRS}
  ${GSTREAMER-APP_LIBRARY_DIRS}
  ${GSTREAMER-AUDIO_LIBRARY_DIRS}
  ${GSTREAMER-PBUTILS_LIBRARY_DIRS}
  ${GSTREAMER-FFT_LIBRARY_DIRS}
)

set(DRIVER_TEMPLATE_SRC
    ${DRIVER_DIR_SRC}/GSTTestApp.cpp)

set(DRIVER_TEMPLATE_INC
    ${DRIVER_DIR_INC}/GSTTestApp.h)

add_library(${DRIVER_TEMPLATE_TARGET_NAME}
            SHARED
            ${DRIVER_TEMPLATE_SRC}
            ${DRIVER_TEMPLATE_PLATFORM_SRC}
            ${DRIVER_TEMPLATE_INC})

target_include_directories(${DRIVER_TEMPLATE_TARGET_NAME}
        PRIVATE
        ${DRIVER_DIR_INC})

