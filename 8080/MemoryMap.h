#pragma once

#include <string>
#include <map>

namespace emul
{
	struct MemoryMapItem
	{
		ADDRESS address = 0;
		std::string label;
		std::string module;
	};

	class MemoryMap : public Logger
	{
	public:
		MemoryMap();
		~MemoryMap();

		MemoryMapItem* Get(const char* label);
		MemoryMapItem* Get(WORD address);

		void Add(MemoryMapItem);

		std::map<WORD, MemoryMapItem> m_addressMap;
		std::map<std::string, MemoryMapItem> m_labelMap;
	};
}
