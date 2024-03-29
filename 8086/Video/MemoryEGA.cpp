#include "stdafx.h"

#include "MemoryEGA.h"
#include "GraphControllerEGA.h"
#include "SequencerEGA.h"
#include <Config.h>

using emul::ADDRESS;
using emul::GetBit;
using emul::SetBit;
using cfg::CONFIG;

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

		EnableLog(CONFIG().GetLogLevel("memory.ega"));
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

		const BYTE* newA = m_planes[2].getPtr() + ((size_t)selectA * 16384);
		const BYTE* newB = m_planes[2].getPtr() + ((size_t)selectB * 16384);

		if (m_charMapA != newA || m_charMapB != newB)
		{
			LogPrintf(LOG_INFO, "SelectCharMaps(adjusted for RAM size), A=[%d], B=[%d]", selectA, selectB);
			m_charMapA = newA;
			m_charMapB = newB;
		}
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

	BYTE MemoryEGA::read(ADDRESS offset) const
	{
		if (!m_enable)
			return 0xFF;

		BYTE selectedPlane = m_graphData->readPlaneSelect;

		// Odd/Even
		if (m_seqData->memoryMode.oddEven)
		{
			SetBit(selectedPlane, 0, GetBit(offset, 0));
		}

		// Chain Odd/Even
		if (m_graphData->chainOddEven)
		{
			SetBit(offset, 0, GetBit(offset, m_ramSize == RAMSIZE::EGA_64K ? 13 : 15));
		}

		offset &= m_planeAddressMask;

		for (int i = 0; i < 4; ++i)
		{
			m_dataLatches[i] = m_planes[i].read(offset);
		}

		if (m_graphData->readModeCompare)
		{
			LogPrintf(LOG_TRACE, "Read[compare][%04x]", offset);
			BYTE compare = 0xFF;
			for (int i = 0; i < 4; ++i)
			{
				// Skip the don't care planes
				if (GetBit(m_graphData->colorDontCare, i))
				{
					BYTE value = m_dataLatches[i];
					BYTE compareWith = GetBit(m_graphData->colorCompare, i) ? 0xFF : 0;
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

		LogPrintf(LOG_TRACE, "Write(%d)[%04x] = %02x", m_graphData->writeMode, offset, data);

		// Odd/Even
		BYTE planeMask = m_seqData->planeMask;
		if (m_seqData->memoryMode.oddEven)
		{
			bool even = !GetBit(offset, 0);
			{
				SetBit(planeMask, 0 + even, false);
				SetBit(planeMask, 2 + even, false);
			}
			SetBit(offset, 0, false);
		}

		// Chain Odd/Even
		if (m_graphData->chainOddEven)
		{
			SetBit(offset, 0, GetBit(offset, m_ramSize == RAMSIZE::EGA_64K ? 13 : 15));
		}

		offset &= m_planeAddressMask;

		if (m_graphData->writeMode == 1)
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
					if (m_graphData->writeMode == 2)
					{
						toWrite = GetBit(data, i) ? 0xFF : 0;
					}
					else // Mode 0
					{
						// Set/Reset
						if (GetBit(m_graphData->enableSetReset, i))
						{
							toWrite = GetBit(m_graphData->setReset, i) ? 0xFF : 0;
						}
						else
						{
							// Rotate
							toWrite = RotateRight(data, m_graphData->rotateCount);
						}
					}

					// ALU
					switch (m_graphData->aluFunction)
					{
					case ALUFunction::AND: toWrite &= m_dataLatches[i]; break;
					case ALUFunction::OR:  toWrite |= m_dataLatches[i]; break;
					case ALUFunction::XOR: toWrite ^= m_dataLatches[i]; break;
					}

					// Bit Mask
					const BYTE& latched = m_dataLatches[i];
					BYTE masked = latched ^ ((toWrite ^ latched) & m_graphData->bitMask);

					m_planes[i].write(offset, masked);
				}
			}
		}
	}

	void MemoryEGA::Serialize(json& to)
	{
		to["ramSize"] = m_ramSize;

		// Plane data will be saved as part of main memory dump.
		// Char maps are initialized by parent

		to["enable"] = m_enable;
		to["dataLatches"] = m_dataLatches;
	}

	void MemoryEGA::Deserialize(const json& from)
	{
		if (from["ramSize"] != m_ramSize)
		{
			throw emul::SerializableException("MemoryEGA: Incompatible ramsize", emul::SerializationError::COMPAT);
		}

		m_enable = from["enable"];
		m_dataLatches = from["dataLatches"];
	}
}
