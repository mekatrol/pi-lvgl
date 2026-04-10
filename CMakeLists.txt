cmake_minimum_required(VERSION 3.16)
project(lvgl_touch_min C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps CACHE PATH "" FORCE)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(PkgConfig REQUIRED)
pkg_check_modules(EVDEV REQUIRED libevdev)
pkg_check_modules(LIBDRM REQUIRED libdrm)

include(FetchContent)
FetchContent_Declare(
  lvgl
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG master
)
FetchContent_GetProperties(lvgl)
if(NOT lvgl_POPULATED)
  FetchContent_Populate(lvgl)
endif()

set(LVGL_SRC_DIR ${lvgl_SOURCE_DIR})
set(LV_CONF_PATH ${CMAKE_BINARY_DIR}/lv_conf.h)

execute_process(
  COMMAND ${Python3_EXECUTABLE}
          ${LVGL_SRC_DIR}/scripts/generate_lv_conf.py
          --template ${LVGL_SRC_DIR}/lv_conf_template.h
          --defaults ${CMAKE_SOURCE_DIR}/lv_conf.defaults
          --config ${LV_CONF_PATH}
  RESULT_VARIABLE GEN_RES
)
if(NOT GEN_RES EQUAL 0)
  message(FATAL_ERROR "Failed to generate lv_conf.h")
endif()

set(LV_BUILD_CONF_PATH ${LV_CONF_PATH} CACHE PATH "" FORCE)
set(CONFIG_LV_BUILD_DEMOS 0 CACHE STRING "" FORCE)
set(LV_CONF_BUILD_DISABLE_DEMOS ON CACHE BOOL "" FORCE)
set(LV_CONF_BUILD_DISABLE_EXAMPLES ON CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES 0 CACHE STRING "" FORCE)
add_subdirectory(${LVGL_SRC_DIR} ${CMAKE_BINARY_DIR}/lvgl)

# Ensure LVGL internal targets see DRM/evdev headers (e.g. drm.h under /usr/include/libdrm)
target_include_directories(lvgl PUBLIC
  ${LIBDRM_INCLUDE_DIRS}
  ${EVDEV_INCLUDE_DIRS}
)

add_executable(app src/main.c)
target_include_directories(app PRIVATE
  ${LVGL_SRC_DIR}
  ${CMAKE_BINARY_DIR}
  ${EVDEV_INCLUDE_DIRS}
  ${LIBDRM_INCLUDE_DIRS}
)

target_link_libraries(app PRIVATE
  lvgl
  ${EVDEV_LIBRARIES}
  ${LIBDRM_LIBRARIES}
  m
  pthread
  rt
)
