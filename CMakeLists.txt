cmake_minimum_required(VERSION 3.28)
project(Imagination LANGUAGES CXX)

#set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT DAYDREAM)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Extract)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/$<CONFIG>-Windows-x64)
set(OBJ_DIR ${CMAKE_BINARY_DIR}/bin-int/$<CONFIG>-Windows-x64)

add_definitions(-DUNICODE -D_UNICODE)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Includes/*.h)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.cpp)
file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Includes/ImGui/*.cpp)

add_executable(Extract
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${HEADERS}
    ${SOURCES}
    ${IMGUI_SOURCES}
)

set_target_properties(Extract PROPERTIES
    WIN32_EXECUTABLE FALSE
)

target_link_options(Extract PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/SUBSYSTEM:CONSOLE>
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${HEADERS} ${SOURCES})

target_include_directories(Extract PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Includes
)

target_compile_definitions(Extract PUBLIC
    IMGN_PLATFORM_WINDOWS
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    
)

target_link_libraries(Extract PUBLIC
    Vulkan::Vulkan
)

set(PRE_COMPILED ${CMAKE_CURRENT_SOURCE_DIR}/Includes/pch.h)
target_precompile_headers(Extract PRIVATE ${PRE_COMPILED})

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/Sources/VulkanDispatcher.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

message(STATUS "Main file = ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
message(STATUS "Sources = ${SOURCES}")

#file(GLOB_RECURSE ImgnVulkanFiles CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/*.h ${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/*.cpp)
# file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.cpp)
#add_library(ImgnVulkan SHARED ${ImgnVulkanFiles}
#${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/main.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/pch.hpp
#${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/pch.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/framework.hpp
#${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/ImgnVulkan.hpp 
#${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/ImgnVulkan.cpp)
#
#target_include_directories(ImgnVulkan PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan)
#target_compile_features(ImgnVulkan PUBLIC cxx_std_23)
#
#target_compile_definitions(ImgnVulkan PRIVATE IMGN_VULKAN_BUILD_DLL PUBLIC IMGN_VULKAN_PLATFORM_WINDOWS VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1)
#
#target_link_libraries(ImgnVulkan PUBLIC Vulkan::Vulkan)
#
#target_precompile_headers(ImgnVulkan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/DLL/ImgnVulkan/pch.hpp)


add_subdirectory(Imagination)
add_subdirectory(Daydream)
add_subdirectory(ImaginationEditor)

#DLL Renderbackends
