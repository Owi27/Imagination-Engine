cmake_minimum_required(VERSION 3.28)
project(Imagination LANGUAGES CXX)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Imagination)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_UNICODE)

set(PRE_COMPILED ${CMAKE_CURRENT_SOURCE_DIR}/Includes/pch.h)

find_package(Vulkan REQUIRED)

file(GLOB HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Includes/*.h)
file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.cpp)

add_executable(Imagination
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Spike.hlsl
    ${HEADERS}
    ${SOURCES}
)

source_group("Headers" FILES ${HEADERS})
source_group("Sources" FILES ${SOURCES})

target_include_directories(Imagination PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Includes
)

target_compile_definitions(Imagination PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)

target_link_libraries(Imagination PUBLIC
    Vulkan::Vulkan
)

target_precompile_headers(Imagination PRIVATE ${PRE_COMPILED})