#include "MemoryMap.h"

MemoryMap::MemoryMap() : Logger("MMAP")
{
}

MemoryMap::~MemoryMap()
{
}

void MemoryMap::Add(MemoryMapItem item)
{
	if (m_addressMap.find(item.address) != m_addressMap.end())
	{
		LogPrintf(LOG_WARNING, "Label already defined for address: 0x%04X", item.address);
	}

	LogPrintf(LOG_INFO, "Adding Label: 0x%04X -> %s (%s)", item.address, item.label.c_str(), item.module.c_str());
	m_addressMap[item.address] = item;
	m_labelMap[item.label] = item;
}

MemoryMapItem* MemoryMap::Get(const char* label)
{
	if (label != nullptr)
	{
		auto item = m_labelMap.find(label);

		if (item != m_labelMap.end())
		{
			return &(item->second);
		}
	}

	return nullptr;
}
MemoryMapItem* MemoryMap::Get(WORD address)
{
	auto item = m_addressMap.find(address);

	if (item != m_addressMap.end())
	{
		return &(item->second);
	}

	return nullptr;
}
