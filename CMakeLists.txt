cmake_minimum_required(VERSION 3.21) # Minimal for CMAKE_TOOLCHAIN_FILE

# vcpkg
set(VCPKG_ROOT "C:/dev/vcpkg")
set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

# Project paths setup for binary, lib and archive
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/archive)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

project(cs143b) # Solution

add_subdirectory(project1)
add_subdirectory(project2)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT project1) # Set a startup project in Visual Studio IDE







