#include "MemoryEGA.h"
#include "../Config.h"

using emul::ADDRESS;
using cfg::Config;

namespace memory_ega
{
	MemoryEGA::MemoryEGA(RAMSIZE ramsize) :
		MemoryBlock("EGA_RAM"),
		m_ramSize(ramsize),
		m_planeSize((DWORD)ramsize / 4),
		m_planeAddressMask(m_planeSize - 1),
		m_planes{
			MemoryBlock("BP0", (DWORD)ramsize / 4), 
			MemoryBlock("BP1", (DWORD)ramsize / 4),
			MemoryBlock("BP2", (DWORD)ramsize / 4),
			MemoryBlock("BP3", (DWORD)ramsize / 4)
		}
	{
		m_size = (DWORD)ramsize;
		SelectCharMaps(0, 0);

		EnableLog(Config::Instance().GetLogLevel("memory.ega"));
	}

	void MemoryEGA::SelectCharMaps(BYTE selectA, BYTE selectB)
	{
		LogPrintf(LOG_DEBUG, "SelectCharMaps, A=[%d], B=[%d]", selectA, selectB);

		switch (m_ramSize)
		{
		case RAMSIZE::EGA_64K:
			selectA = 0;
			selectB = 0;
			break;

		case RAMSIZE::EGA_128K:
			selectA &= 1;
			selectB &= 1;
			break;

		case RAMSIZE::EGA_256K:
			selectA &= 3;
			selectB &= 3;
			break;
		}

		LogPrintf(LOG_INFO, "SelectCharMaps(adjusted for RAM size), A=[%d], B=[%d]", selectA, selectB);
		m_charMapA = m_planes[2].getPtr() + ((size_t)selectA * 16384);
		m_charMapB = m_planes[2].getPtr() + ((size_t)selectB * 16384);
	}

	// TODO, odd-even, compare
	BYTE MemoryEGA::read(ADDRESS offset) 
	{
		if (!m_enable)
			return 0xFF;

		LogPrintf(LOG_DEBUG, "read[%04x]", offset);
		if (m_readModeCompare)
		{
			LogPrintf(LOG_ERROR, "Read Mode Compare not implemented");
		}
		return m_planes[m_currReadPlane].read(offset & m_planeAddressMask);
	}

	void MemoryEGA::write(ADDRESS offset, BYTE data) 
	{
		if (!m_enable)
			return;

		LogPrintf(LOG_DEBUG, "write[%04x]=%02x", offset, data);
		m_planes[0].write(offset & m_planeAddressMask, data);
	}
}
