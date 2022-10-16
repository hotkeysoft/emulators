#pragma once

#include <FileUtil.h>

namespace emul
{
	class PRGLoader
	{
	public:
		virtual void LoadPRG(const hscommon::fileUtil::PathList& paths) = 0;

		virtual std::string GetPRGInfo() const { return ""; }

		virtual bool CanUnloadPRG() const { return false; };
		virtual void UnloadPRG() {};
	};
}