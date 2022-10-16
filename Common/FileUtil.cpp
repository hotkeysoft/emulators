#include "stdafx.h"

#include <FileUtil.h>
#include <commdlg.h>

#include <SDL.h>
#include <SDL_syswm.h>

#ifndef HSCOMMON_NO_MAINWINDOW
#include <UI/MainWindow.h>
using ui::MAINWND;
#endif

namespace fs = std::filesystem;

namespace hscommon::fileUtil
{
	HWND GetHWND()
	{
#ifdef HSCOMMON_NO_MAINWINDOW
		return nullptr;
#else
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(MAINWND().GetWindow(), &wmInfo);
		return wmInfo.info.win.window;
#endif
	}

	bool SelectFile(fs::path& path, SelectFileFilters filters, bool addAllFilesFilter)
	{
		std::string filterStr;
		if (addAllFilesFilter)
		{
			filters.push_back(std::make_pair("All Files (*.*)", "*.*"));
		}
		for (auto& filter : filters)
		{
			filterStr.append(filter.first);
			filterStr.push_back('\0');
			filterStr.append(filter.second);
			filterStr.push_back('\0');
		}

		OPENFILENAMEA ofn;
		char szFile[32000];

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = GetHWND();
		ofn.lpstrFile = szFile;
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
		// use the contents of szFile to initialize itself.
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filterStr.c_str();
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		// Display the Open dialog box.

		if (!GetOpenFileNameA(&ofn))
		{
			return false;
		}
		path = szFile;
		return true;
	}
}
