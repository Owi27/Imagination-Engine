project(ImaginationEditor)

file(GLOB_RECURSE ImaginationEditor_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE ImaginationEditor_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(ImaginationEditor ${ImaginationEditor_HEADERS} ${ImaginationEditor_SOURCES})

target_precompile_headers(ImaginationEditor PRIVATE ../Imagination/src/pch.hpp)
target_include_directories(ImaginationEditor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Imagination/src)

target_link_libraries(ImaginationEditor PRIVATE IMGN)

target_compile_definitions(ImaginationEditor PRIVATE IMGN_PLATFORM_WINDOWS)

set_target_properties(ImaginationEditor PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/ImaginationEditor LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/ImaginationEditor ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/ImaginationEditor OBJECT_OUTPUTS_DIRECTORY ${OBJ_DIR}/ImaginationEditor)
