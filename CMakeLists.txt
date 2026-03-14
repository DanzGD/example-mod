cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)          # Upgrade ke C++23 untuk fitur modern
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Architecture handling
if(IOS OR "${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Project name sesuai mod ID (tanpa com.danz.)
project(GDProgressBot VERSION 1.0.0)

# Sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h)  # Tambah .h kalau ada header

# Mod library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Geode SDK check
if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK environment variable not set! Point it to your Geode SDK path.")
endif()

message(STATUS "Using Geode SDK: $ENV{GEODE_SDK}")

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Setup mod (ini yang handle bindings, resources, dll)
setup_geode_mod(${PROJECT_NAME})