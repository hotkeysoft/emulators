#include "MemoryEGA.h"
#include "GraphControllerEGA.h"
#include "../Config.h"

using emul::ADDRESS;
using emul::GetBit;
using emul::SetBit;
using cfg::Config;

using graph_ega::ALUFunction;

namespace memory_ega
{
	std::string PathAppendIndex(const char* inPath, int index)
	{
		std::string path(inPath);
		std::ostringstream os;
		os << "_" << index;
		auto pos = path.find_last_of('.');
		path.insert(pos, os.str());
		return path;
	}

	BYTE RotateRight(BYTE b, int count)
	{
		return (b >> count) | (b << (8 - count));
	}

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

	void MemoryEGA::Clear(BYTE filler)
	{
		for (int i = 0; i < 4; ++i)
		{
			m_planes[i].Clear();
		}
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

	bool MemoryEGA::LoadFromFile(const char* file, WORD offset)
	{
		for (int i = 0; i < 4; ++i)
		{
			std::string planeFile = PathAppendIndex(file, i);

			if (!m_planes[i].LoadFromFile(planeFile.c_str()))
			{
				return false;
			}
		}
		return true;
	}

	bool MemoryEGA::Dump(emul::ADDRESS, emul::DWORD, const char* outFile) const
	{
		for (int i = 0; i < 4; ++i)
		{
			std::string planeFile = PathAppendIndex(outFile, i);
			m_planes[i].Dump(0, 0, planeFile.c_str());
		}
		return true;
	}

	BYTE MemoryEGA::read(ADDRESS offset) 
	{
		if (!m_enable)
			return 0xFF;

		BYTE selectedPlane = m_graphCtrl->readPlaneSelect;
		// Odd/Even
		if (m_graphCtrl->oddEven)
		{
			SetBit(selectedPlane, 0, GetBit(offset, 0));
		}

		// Chain Odd/Even
		if (m_graphCtrl->chainOddEven)
		{
			SetBit(offset, 0, GetBit(offset, m_ramSize == RAMSIZE::EGA_64K ? 13 : 15));
		}

		offset &= m_planeAddressMask;

		for (int i = 0; i < 4; ++i)
		{
			m_dataLatches[i] = m_planes[i].read(offset);
		}

		if (m_graphCtrl->readModeCompare)
		{
			LogPrintf(LOG_TRACE, "Read[compare][%04x]", offset);
			BYTE compare = 0xFF;
			for (int i = 0; i < 4; ++i)
			{
				// Skip the don't care planes
				if (GetBit(m_graphCtrl->colorDontCare, i))
				{
					BYTE value = m_dataLatches[i];
					BYTE compareWith = GetBit(m_graphCtrl->colorCompare, i) ? 0xFF : 0;
					BYTE match = ~(value ^ compareWith);
					compare &= match;
				}
			}
			return compare;
		}
		else
		{
			LogPrintf(LOG_TRACE, "Read[%04x]", offset);
			return m_dataLatches[selectedPlane];
		}
	}

	void MemoryEGA::write(ADDRESS offset, BYTE data) 
	{
		if (!m_enable)
			return;

		LogPrintf(LOG_TRACE, "Write(%d)[%04x] = %02x", m_graphCtrl->writeMode, offset, data);

		// Odd/Even
		BYTE planeMask = m_planeMask;
		if (m_graphCtrl->oddEven)
		{
			bool even = !GetBit(offset, 0);
			{
				SetBit(planeMask, 0 + even, false);
				SetBit(planeMask, 2 + even, false);
			}
			SetBit(offset, 0, false);
		}

		// Chain Odd/Even
		if (m_graphCtrl->chainOddEven)
		{
			SetBit(offset, 0, GetBit(offset, m_ramSize == RAMSIZE::EGA_64K ? 13 : 15));
		}

		offset &= m_planeAddressMask;

		if (m_graphCtrl->writeMode == 1)
		{
			for (int i = 0; i < 4; ++i)
			{
				if (GetBit(planeMask, i))
				{
					m_planes[i].write(offset, m_dataLatches[i]);
				}
			}
		}
		else // Modes 0 & 2
		{
			for (int i = 0; i < 4; ++i)
			{
				if (GetBit(planeMask, i))
				{
					BYTE toWrite;
					if (m_graphCtrl->writeMode == 2)
					{
						toWrite = GetBit(data, i) ? 0xFF : 0;
					}
					else // Mode 0
					{
						// Set/Reset
						if (GetBit(m_graphCtrl->enableSetReset, i))
						{
							toWrite = GetBit(m_graphCtrl->setReset, i) ? 0xFF : 0;
						}
						else
						{
							// Rotate
							toWrite = RotateRight(data, m_graphCtrl->rotateCount);
						}
					}				

					// ALU
					switch (m_graphCtrl->aluFunction)
					{
					case ALUFunction::AND: toWrite &= m_dataLatches[i]; break;
					case ALUFunction::OR:  toWrite |= m_dataLatches[i]; break;
					case ALUFunction::XOR: toWrite ^= m_dataLatches[i]; break;
					}

					// Bit Mask
					toWrite &= m_graphCtrl->bitMask; // Clear locked bits
					BYTE locked = m_dataLatches[i] & (~m_graphCtrl->bitMask); // Put latched value in locked bits
					m_planes[i].write(offset, toWrite | locked);
				}
			}
		}
	}
}
