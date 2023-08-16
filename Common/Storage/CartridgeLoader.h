#pragma once

#include <FileUtil.h>

namespace emul
{
	class CartridgeLoader
	{
	public:
		virtual void LoadCartridge(const std::filesystem::path& path) = 0;

		virtual hscommon::fileUtil::SelectFileFilters GetLoadFilter() { return { {"Cartridge ROM (*.rom *.bin)", "*.rom;*.bin"} }; }

		virtual std::string GetCartridgeInfo() const = 0;

		virtual bool CanUnloadCartridge() const { return true; };
		virtual void UnloadCartridge() {};
	};
}