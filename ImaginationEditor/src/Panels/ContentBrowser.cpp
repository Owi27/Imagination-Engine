#include "pch.hpp"
#include "ContentBrowser.hpp"

#include <ImGui/imgui.h>

namespace Imgn
{
	//change on projects
	void ContentBrowser::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (_currentDirectory != FileSystem::Assets())
		{
			if (ImGui::Button("<"))
			{
				_currentDirectory = _currentDirectory.parent_path();
			}
		}

		for (auto& directory : std::filesystem::directory_iterator(_currentDirectory))
		{
			auto relativePath = std::filesystem::relative(directory.path(), FileSystem::Assets());
			std::string filenameString = relativePath.filename().string();
			bool clicked = ImGui::Button(filenameString.c_str());

			if (ImGui::BeginDragDropSource())
			{
				std::string itemPath = relativePath.string();
				ImGui::SetDragDropPayload("ContentBrowserItem", itemPath.data(), itemPath.size(), ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			if (clicked && directory.is_directory())    
			{
				_currentDirectory /= directory.path().filename();
			}
		}

		ImGui::End();
	}
}