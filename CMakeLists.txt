cmake_minimum_required(VERSION 2.6)

project(BEAE)

set(CMAKE_MODULE_PATH "${BEAE_SOURCE_DIR}/CMakeModules")

include(UseHaiku)

add_subdirectory(Core-src)

