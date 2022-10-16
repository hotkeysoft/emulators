#pragma once

namespace emul
{
	class PRGLoader
	{
	public:
		virtual void LoadPRG(const char* path) = 0;
	};
}