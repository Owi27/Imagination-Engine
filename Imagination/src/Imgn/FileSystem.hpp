#pragma once

#include "ImgnCore.hpp"

#include <filesystem>

namespace Imgn
{
	class IMGN_API FileSystem
	{
		static std::filesystem::path FindEngineRoot();
		//static std::filesystem::path GetExecutableDirectory();

	public:
		static const std::filesystem::path& GetEngineRoot();
		static const std::filesystem::path Assets();
	};
}