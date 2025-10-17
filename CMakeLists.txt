cmake_minimum_required(VERSION 3.5)
Project(Imagination)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(Vulkan REQUIRED)
add_subdirectory(imaginations)
add_subdirectory(ImaginaryMathLibrary)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(Imagination ${SOURCES})

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h ${CMAKE_CURRENT_SOURCE_DIR}/includes/*.hpp)
source_group("Headers" FILES ${HEADERS})
target_sources(Imagination PRIVATE ${HEADERS})

target_include_directories(Imagination PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes/ ${PROJECT_SOURCE_DIR}/ImaginaryMathLibrary ${Vulkan_INCLUDE_DIRS})

target_link_libraries(Imagination PUBLIC ImaginaryMathLibrary ${Vulkan_LIBRARIES})