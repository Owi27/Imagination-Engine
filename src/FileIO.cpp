#include "D:/GitHub/Imagination-Engine/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "FileIO.h"
#include <filesystem>

RETURN(size_t) FileIO::GetFileSize()
{
	return std::filesystem::file_size(_filePath);
}
