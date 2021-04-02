# Copyright 2018 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.5.2)

project(flutter_embeder)

set(MYARM_TOOLCHAIN $ENV{MYARM_TOOLCHAIN})
if ( "$ENV{MYARM_TOOLCHAIN}" STREQUAL "" )
  message(FATAL_ERROR, " Please set MYARM_TOOLCHAIN and try again.")
  return()
endif()
set(FLUTTER_ENGINE_ROOT $ENV{FLUTTER_ENGINE_ROOT})
if ( "$ENV{FLUTTER_ENGINE_ROOT}" STREQUAL "" )
  message(FATAL_ERROR, " Please set FLUTTER_ENGINE_ROOT and try again.")
  return()
endif()
set(FLUTTER_ENGINE_DIR $ENV{FLUTTER_ENGINE_DIR})
if ( "$ENV{FLUTTER_ENGINE_DIR}" STREQUAL "" )
  message(FATAL_ERROR, " Please set FLUTTER_ENGINE_DIR and try again.")
  return()
endif()

# Cross compiler
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm64)
set(triple aarch64-linux-gnu)

set(tools ${MYARM_TOOLCHAIN}/bin)
set(CMAKE_C_COMPILER ${tools}/aarch64-buildroot-linux-gnu-gcc)
set(CMAKE_C_LINKER ${tools}/aarch64-buildroot-linux-gnu-gcc)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER ${tools}/aarch64-buildroot-linux-gnu-g++)
set(CMAKE_CXX_LINKER ${tools}/aarch64-buildroot-linux-gnu-g++)
set(CMAKE_CXX_COMPILER_TARGET ${triple})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 ")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 ")

# Executable
set(FLUTTER_WAYLAND_SRC
  ${CMAKE_SOURCE_DIR}/src/main.cc
  ${CMAKE_SOURCE_DIR}/flutter/standard_codec.cc
  ${CMAKE_SOURCE_DIR}/src/flutter_application.cc
  ${CMAKE_SOURCE_DIR}/src/wayland_display.cc
  ${CMAKE_SOURCE_DIR}/src/utils.cc
  ${CMAKE_SOURCE_DIR}/src/input_hook.cc
  ${CMAKE_SOURCE_DIR}/src/platform_channel.cc
)

set(SYSROOT ${MYARM_TOOLCHAIN}/aarch64-buildroot-linux-gnu/sysroot/)

link_directories(
	${CMAKE_BINARY_DIR}
	${SYSROOT}/usr/lib/aarch64-linux-gnu
	${SYSROOT}/lib/aarch64-linux-gnu
	${SYSROOT}/usr/lib/
	${SYSROOT}/lib/
	${FLUTTER_ENGINE_ROOT}/lib
)

add_executable(flutter_embeder ${FLUTTER_WAYLAND_SRC})

target_link_libraries(flutter_embeder
  wayland-egl
  wayland-client
  EGL
  flutter_engine
  xkbcommon
  input
  udev
  pthread
)

target_include_directories(flutter_embeder
  PRIVATE
  src/
  ./
  ${SYSROOT}/usr/include
  ${SYSROOT}/usr/include/drm
  ${FLUTTER_ENGINE_ROOT}/include
  ${FLUTTER_ENGINE_DIR}/third_party/rapidjson/include/
)

