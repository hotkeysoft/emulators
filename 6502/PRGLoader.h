#pragma once

#include <FileUtil.h>

namespace emul
{
	class PRGLoader
	{
	public:
		virtual void LoadPRG(const hscommon::fileUtil::PathList& paths) = 0;
	};
}