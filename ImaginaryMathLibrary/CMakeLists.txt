cmake_minimum_required(VERSION 3.5)
add_library(ImaginaryMathLibrary ImaginaryMath.cpp ImaginaryMath.hpp)

target_include_directories(ImaginaryMathLibrary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})