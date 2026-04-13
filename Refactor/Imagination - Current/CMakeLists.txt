cmake_minimum_required(VERSION 3.28)
project(Imagination LANGUAGES CXX)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Imagination)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_UNICODE)

set(PRE_COMPILED ${CMAKE_CURRENT_SOURCE_DIR}/Includes/pch.h)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Includes/*.h)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.cpp)
file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Includes/ImGui/*.cpp)

add_executable(Imagination
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Spike.hlsl
    ${HEADERS}
    ${SOURCES}
    ${IMGUI_SOURCES}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Spike.hlsl ${HEADERS} ${SOURCES})

target_include_directories(Imagination PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Includes
)

target_compile_definitions(Imagination PUBLIC
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)

target_link_libraries(Imagination PUBLIC
    Vulkan::Vulkan
)

target_precompile_headers(Imagination PRIVATE ${PRE_COMPILED})

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/Sources/VulkanDispatcher.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

#DLL Renderbackends
add_library(ImgnVulkan SHARED 
${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/main.cpp
${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/pch.hpp
${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/pch.cpp
${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/framework.hpp
${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/ImgnVulkan.hpp 
${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/ImgnVulkan.cpp)

target_compile_features(ImgnVulkan PUBLIC cxx_std_23)

target_compile_definitions(ImgnVulkan PUBLIC VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1)

target_link_libraries(ImgnVulkan PUBLIC Vulkan::Vulkan)

target_precompile_headers(ImgnVulkan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/pch.hpp)
