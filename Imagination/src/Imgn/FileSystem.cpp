#include "pch.hpp"
#include "FileSystem.hpp"

namespace Imgn
{
	std::filesystem::path FileSystem::FindEngineRoot()
	{
		std::filesystem::path directory = std::filesystem::current_path();

		while (!directory.empty())
		{
			bool hasAssets = std::filesystem::exists(directory / "Assets");
			bool hasEngine = std::filesystem::exists(directory / "Imagination");
			bool hasEditor = std::filesystem::exists(directory / "ImaginationEditor");

			if (hasAssets && hasEngine && hasEditor) return directory;

			std::filesystem::path parent = directory.parent_path();

			if (parent == directory) break;

			directory = parent;
		}

		IMGN_ERROR("Could not find Imagination Engine root");
		return {};
	}

	const std::filesystem::path& FileSystem::GetEngineRoot()
	{
		static const std::filesystem::path engineRoot = FindEngineRoot();

		return engineRoot;
	}

	const std::filesystem::path FileSystem::Assets()
	{
		return GetEngineRoot() / "Assets";
	}
}