#include "MemoryEGA.h"

using emul::ADDRESS;
using emul::BytePtr;

namespace memory_ega
{
	MemoryEGA::MemoryEGA(RAMSIZE ramsize) :
		MemoryBlock("EGA_RAM"),
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
	}

	// TODO, odd-even, compare
	BYTE MemoryEGA::read(ADDRESS offset) 
	{
		LogPrintf(LOG_INFO, "read[%04x]", offset);
		if (m_readModeCompare)
		{
			LogPrintf(LOG_ERROR, "Read Mode Compare not implemented");
		}
		return m_planes[m_currReadPlane].read(offset & m_planeAddressMask);
	}

	BytePtr MemoryEGA::getPtr(ADDRESS offset)
	{
		return m_planes[0].getPtr(offset & m_planeAddressMask);
	}

	void MemoryEGA::write(ADDRESS offset, BYTE data) 
	{
		LogPrintf(LOG_INFO, "write[%04x]=%02x", offset, data);
		m_planes[0].write(offset & m_planeAddressMask, data);
	}
}
