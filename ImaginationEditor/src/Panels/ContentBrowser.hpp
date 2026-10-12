#pragma once
#include <Imgn/FileSystem.hpp>

namespace Imgn
{
	class ContentBrowser
	{
		std::filesystem::path _currentDirectory;

	public:
		ContentBrowser()
		{
			_currentDirectory = FileSystem::Assets();
		}

		void OnImGuiRender();
	};
}