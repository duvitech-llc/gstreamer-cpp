cmake_minimum_required(VERSION 3.9.0)
project(GSTPipelineApp)

find_package(PkgConfig REQUIRED)

set(GSTREAMER_MINIMUM_VERSION 1.0.5)

pkg_check_modules(GST1_TEST gstreamer-1.0)
if ( GST1_TEST_FOUND AND NOT ${GST1_TEST_VERSION} VERSION_LESS ${GSTREAMER_MINIMUM_VERSION} )
    pkg_check_modules(GSTLIBS REQUIRED
            gobject-2.0
            glib-2.0
            gstreamer-1.0
            gstreamer-app-1.0
            gstreamer-pbutils-1.0
            json-glib-1.0
            gstreamer-check-1.0)

endif()

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(DRIVER_DIR_SRC                 src)
set(DRIVER_DIR_INC                 include)
set(DRIVER_TEMPLATE_TARGET_NAME    "GSTPipelineApp")

include_directories(
  ${GSTLIBS_INCLUDE_DIRS}
  ${DRIVER_DIR_INC}
)

set(SOURCE_FILES src/gstPipelineApp.cxx)

link_directories(
  ${GSTLIBS_LIBRARY_DIRS}
)

include_directories(
  $GSTLIBS_INCLUDE_DIRS}
  ${DRIVER_DIR_INC}
)

link_directories(
  ${GSTLIBS_LIBRARY_DIRS}
)


add_library(${DRIVER_TEMPLATE_TARGET_NAME}
            SHARED
            src/gstPipelineApp.cxx)

target_include_directories(${DRIVER_TEMPLATE_TARGET_NAME}
        PRIVATE
        ${DRIVER_DIR_INC})

