#pragma once
#include <string>
#include <Gateware/Gateware.h>

#include <Imgn/ImgnApp.hpp>

#ifdef IMGN_PLATFORM_WINDOWS
#include <commdlg.h>
#endif // IMGN_PLATFORM_WINDOWS

namespace Imgn
{
	namespace FileDialogs
	{
		static std::string OpenFile(const char* pFilter)
		{
#ifdef IMGN_PLATFORM_WINDOWS
			OPENFILENAMEA ofn;
			char szFile[256] = { 0 };
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = static_cast<HWND>(ImgnApp::Get().GetWindow().GetWindowHandle());
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = pFilter;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

			if (GetOpenFileNameA(&ofn) == TRUE) return ofn.lpstrFile;
#endif // IMGN_PLATFORM_WINDOWS

			return std::string();
		}

		static std::string SaveFile(const char* pFilter)
		{
#ifdef IMGN_PLATFORM_WINDOWS
			OPENFILENAMEA ofn;
			char szFile[256] = { 0 };
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = static_cast<HWND>(ImgnApp::Get().GetWindow().GetWindowHandle());
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = pFilter;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

			if (GetSaveFileNameA(&ofn) == TRUE) return ofn.lpstrFile;
#endif // IMGN_PLATFORM_WINDOWS

			return std::string();
		}
	}
}