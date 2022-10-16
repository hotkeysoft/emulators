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

	using StringBuf = std::vector<char>;

	static bool SelectFile(StringBuf& outFileNames, WORD& outFileOffset, SelectFileFilters filters, bool addAllFilesFilter, bool multi)
	{
		outFileNames.resize(32000);

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
		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = GetHWND();
		ofn.lpstrFile = &outFileNames[0];
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
		// use the contents of szFile to initialize itself.
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = (DWORD)outFileNames.size();
		ofn.lpstrFilter = filterStr.c_str();
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST
			| OFN_FILEMUSTEXIST
			| OFN_NOCHANGEDIR
			| (multi ? (OFN_ALLOWMULTISELECT | OFN_EXPLORER ) : 0);

		outFileOffset = ofn.nFileOffset;

		// Display the Open dialog box.
		return GetOpenFileNameA(&ofn);
	}

	bool SelectFile(fs::path& path, SelectFileFilters filters, bool addAllFilesFilter)
	{
		StringBuf buf;
		WORD fileOffset = 0;
		if (!SelectFile(buf, fileOffset, filters, addAllFilesFilter, false))
		{
			return false;
		}

		path = &buf[0];
		return true;
	}

	bool SelectFileMulti(PathList& paths, SelectFileFilters filters, bool addAllFilesFilter)
	{
		paths.clear();

		StringBuf buf;
		WORD fileOffset = 0;
		if (!SelectFile(buf, fileOffset, filters, addAllFilesFilter, true))
		{
			return false;
		}

		const char* bufStr = &buf[0];

		fs::path basePath(bufStr);
		bufStr += strlen(bufStr) + 1;

		if ((*bufStr) == '\0')
		{
			// Single file, "base Path" is actual full path
			paths.push_back(basePath);
			return true;
		}

		while (*bufStr)
		{
			paths.push_back(basePath / fs::path(bufStr));
			bufStr += strlen(bufStr) + 1;
		}

		return true;
	}

}
