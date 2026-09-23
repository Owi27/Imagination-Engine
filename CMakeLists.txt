cmake_minimum_required(VERSION 3.28)
project(Imagination LANGUAGES CXX)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ImaginationEditor)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/$<CONFIG>-Windows-x64)
set(OBJ_DIR ${CMAKE_BINARY_DIR}/bin-int/$<CONFIG>-Windows-x64)
add_definitions(-DUNICODE -D_UNICODE)

find_package(Vulkan REQUIRED)

add_subdirectory(Imagination)
add_subdirectory(Daydream)
add_subdirectory(ImaginationEditor)
