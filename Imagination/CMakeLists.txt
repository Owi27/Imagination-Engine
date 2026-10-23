project(IMGN)

#include(sources.cmake)

file(GLOB_RECURSE IMGN_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

file(GLOB_RECURSE IMGN_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE IMGN_VULKAN_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Vulkan/*.cpp)

file(GLOB_RECURSE IMGN_METAL_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Metal/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/Metal/*.mm)

list(REMOVE_ITEM IMGN_SOURCES ${IMGN_VULKAN_SOURCES})
list(REMOVE_ITEM IMGN_SOURCES ${IMGN_METAL_SOURCES})

message(PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
message(CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_library(IMGN SHARED ${IMGN_HEADERS} ${IMGN_SOURCES})

if(WIN32)
    target_sources(IMGN PRIVATE ${IMGN_VULKAN_SOURCES})
elseif(APPLE)
    target_sources(IMGN PRIVATE ${IMGN_METAL_SOURCES})
endif()

target_precompile_headers(IMGN PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp)
target_include_directories(IMGN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)

target_compile_definitions(IMGN PRIVATE IMGN_BUILD_DLL)
    
if (WIN32)
    target_compile_definitions(IMGN PUBLIC IMGN_PLATFORM_WINDOWS IMGN_VULKAN_PLATFORM_WINDOWS)
    
    target_link_libraries(IMGN PUBLIC Vulkan::Vulkan comctl32)
elseif(APPLE)
    target_compile_definitions(IMGN PUBLIC IMGN_PLATFORM_MACOS IMGN_METAL_PLATFORM_MACOS)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    
    target_link_libraries(IMGN PUBLIC ${METAL_FRAMEWORK} ${FOUNDATION_FRAMEWORK} ${QUARTZCORE_FRAMEWORK})
    target_compile_options(IMGN PRIVATE -fmodules -fcxx-modules)
endif()
    
if(APPLE)
    set_source_files_properties(${IMGN_SOURCES} ${IMGN_METAL_SOURCES} PROPERTIES LANGUAGE OBJCXX)
endif()

set_target_properties(IMGN PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/Imagination LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/Imagination ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/Imagination OBJECT_OUTPUTS_DIRECTORY ${OBJ_DIR}/Imagination)

add_custom_command(TARGET IMGN POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}/Daydream COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:IMGN>" "${OUTPUT_DIR}/Daydream") # COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:ImgnVulkan>" "${OUTPUT_DIR}/Daydream")
add_custom_command(TARGET IMGN POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}/ImaginationEditor COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:IMGN>" "${OUTPUT_DIR}/ImaginationEditor")

#add_custom_command(TARGET IMGN POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:ImgnVulkan> $<TARGET_FILE_DIR:IMGN>)
if(MSVC)
    target_compile_options(IMGN PRIVATE /Zi)
    target_link_options(IMGN PRIVATE /DEBUG:FULL /INCREMENTAL:NO)
endif()
