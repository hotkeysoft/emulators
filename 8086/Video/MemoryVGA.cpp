#include "stdafx.h"

#include "MemoryVGA.h"
#include "GraphControllerVGA.h"
#include "SequencerVGA.h"
#include "../Config.h"

using emul::ADDRESS;
using emul::GetBit;
using emul::SetBit;
using cfg::CONFIG;

using graph_vga::ALUFunction;

namespace memory_vga
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

	MemoryVGA::MemoryVGA() :
		MemoryBlock("VGA_RAM"),
		m_ramSize(RAMSIZE::VGA_256K),
		m_planeSize((DWORD)m_ramSize / 4),
		m_planeAddressMask(m_planeSize - 1),
		m_planes{
			MemoryBlock("BP0", (DWORD)m_ramSize / 4),
			MemoryBlock("BP1", (DWORD)m_ramSize / 4),
			MemoryBlock("BP2", (DWORD)m_ramSize / 4),
			MemoryBlock("BP3", (DWORD)m_ramSize / 4)
		}
	{
		m_size = (DWORD)m_ramSize;
		SelectCharMaps(0, 0);

		EnableLog(CONFIG().GetLogLevel("memory.vga"));
	}

	void MemoryVGA::Clear(BYTE filler)
	{
		for (int i = 0; i < 4; ++i)
		{
			m_planes[i].Clear();
		}
	}

	void MemoryVGA::SelectCharMaps(BYTE selectA, BYTE selectB)
	{
		selectA &= 3;
		selectB &= 3;

		const BYTE* newA = m_planes[2].getPtr() + ((size_t)selectA * 16384);
		const BYTE* newB = m_planes[2].getPtr() + ((size_t)selectB * 16384);
		
		if (m_charMapA != newA || m_charMapB != newB)
		{
			LogPrintf(LOG_INFO, "SelectCharMaps(adjusted for RAM size), A=[%d], B=[%d]", selectA, selectB);
			m_charMapA = newA;
			m_charMapB = newB;
		}
	}

	bool MemoryVGA::LoadFromFile(const char* file, WORD offset)
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

	bool MemoryVGA::Dump(emul::ADDRESS, emul::DWORD, const char* outFile) const
	{
		for (int i = 0; i < 4; ++i)
		{
			std::string planeFile = PathAppendIndex(outFile, i);
			m_planes[i].Dump(0, 0, planeFile.c_str());
		}
		return true;
	}

	BYTE MemoryVGA::read(ADDRESS offset)
	{
		if (!m_enable)
			return 0xFF;

		BYTE selectedPlane = m_graphData->readPlaneSelect;
		if (m_seqData->memoryMode.chain4)
		{
			selectedPlane = offset & 3;

			SetBit(offset, 0, GetBit(offset, 14));
			SetBit(offset, 1, GetBit(offset, 15));
		}
		else if (m_seqData->memoryMode.oddEven)
		{
			SetBit(selectedPlane, 0, GetBit(offset, 0));
		}

		// Chain Odd/Even
		if (m_graphData->chainOddEven)
		{
			SetBit(offset, 0, GetBit(offset, 15));
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

	void MemoryVGA::write(ADDRESS offset, BYTE data)
	{
		if (!m_enable)
			return;

		LogPrintf(LOG_TRACE, "Write(%d)[%04x] = %02x", m_graphData->writeMode, offset, data);

		BYTE planeMask = m_seqData->planeMask;
		if (m_seqData->memoryMode.chain4)
		{
			BYTE plane = 0;
			SetBit(plane, offset & 3, true);
			planeMask &= plane;

			SetBit(offset, 0, GetBit(offset, 14));
			SetBit(offset, 1, GetBit(offset, 15));
		}
		else if (m_seqData->memoryMode.oddEven)
		{
			bool even = !GetBit(offset, 0);
			SetBit(planeMask, 0 + even, false);
			SetBit(planeMask, 2 + even, false);
			SetBit(offset, 0, false);
		}

		// Chain Odd/Even
		if (m_graphData->chainOddEven)
		{
			SetBit(offset, 0, GetBit(offset, 15));
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
		else // Modes 0,2,3
		{
			BYTE mask = m_graphData->bitMask;
			if (m_graphData->writeMode == 3)
			{
				mask &= RotateRight(data, m_graphData->rotateCount);
			}

			for (int i = 0; i < 4; ++i)
			{
				if (GetBit(planeMask, i))
				{
					BYTE toWrite;
					if (m_graphData->writeMode == 2)
					{
						toWrite = GetBit(data, i) ? 0xFF : 0;
					}
					else // Mode 0, 3
					{
						// Set/Reset
						if ((m_graphData->writeMode == 3) || GetBit(m_graphData->enableSetReset, i))
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
					BYTE masked = latched ^ ((toWrite ^ latched) & mask);

					m_planes[i].write(offset, masked);
				}
			}
		}
	}

	void MemoryVGA::Serialize(json& to)
	{
		to["ramSize"] = m_ramSize;

		// Plane data will be saved as part of main memory dump.
		// Char maps are initialized by parent

		to["enable"] = m_enable;
		to["dataLatches"] = m_dataLatches;
	}

	void MemoryVGA::Deserialize(const json& from)
	{
		if (from["ramSize"] != m_ramSize)
		{
			throw emul::SerializableException("MemoryVGA: Incompatible ramsize", emul::SerializationError::COMPAT);
		}

		m_enable = from["enable"];
		m_dataLatches = from["dataLatches"];
	}
}
