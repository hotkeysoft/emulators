#include "stdafx.h"

#include "GraphControllerVGA.h"
#include "../CPU/MemoryBlock.h"
#include "../CPU/Memory.h"

using emul::GetBit;

namespace graph_vga
{
	GraphController::GraphController(WORD baseAddress) :
		Logger("graphVGA"),
		m_baseAddress(baseAddress)
	{
	}

	void GraphControllerData::Reset()
	{
		memset(this, 0, sizeof(GraphControllerData));
	}

	void GraphController::Reset()
	{
		m_data.Reset();
		m_currAddress = GraphControllerAddress::GRAPH_INVALID_REG;
		DisconnectPorts();
		MapMemory();
	}

	void GraphController::Init(emul::Memory* memory, emul::MemoryBlock* vgaRAM)
	{
		assert(memory);
		m_memory = memory;
		
		assert(vgaRAM);
		m_vgaRAM = vgaRAM;

		Reset();
		MapMemory();
	}

	void GraphController::ConnectPorts()
	{
		Connect(m_baseAddress + 0xC, static_cast<PortConnector::OUTFunction>(&GraphController::WritePosition1));
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::OUTFunction>(&GraphController::WritePosition2));

		Connect(m_baseAddress + 0xE, static_cast<PortConnector::INFunction>(&GraphController::ReadAddress));
		Connect(m_baseAddress + 0xE, static_cast<PortConnector::OUTFunction>(&GraphController::WriteAddress));

		Connect(m_baseAddress + 0xF, static_cast<PortConnector::INFunction>(&GraphController::ReadValue));
		Connect(m_baseAddress + 0xF, static_cast<PortConnector::OUTFunction>(&GraphController::WriteValue));
	}
	void GraphController::DisconnectPorts()
	{
		DisconnectOutput(m_baseAddress + 0xC);
		DisconnectOutput(m_baseAddress + 0xA);

		DisconnectInput(m_baseAddress + 0xE);
		DisconnectOutput(m_baseAddress + 0xE);

		DisconnectInput(m_baseAddress + 0xF);
		DisconnectOutput(m_baseAddress + 0xF);
	}

	void GraphController::WritePosition1(BYTE value)
	{
		// Not really used
		LogPrintf(Logger::LOG_INFO, "WritePosition1, value=%02Xh", value);
		if ((value & 3) != 0)
		{
			LogPrintf(Logger::LOG_WARNING, "WritePosition1, expected value=0");
			throw std::exception("WritePosition1, expected value=0");
		}
	}
	void GraphController::WritePosition2(BYTE value)
	{
		// TODO Not sure how this works
		LogPrintf(Logger::LOG_INFO, "WritePosition2, value=%02Xh", value);
		if ((value & 3) != 1)
		{
			LogPrintf(Logger::LOG_WARNING, "WritePosition2, expected value=1");
			throw std::exception("WritePosition2, expected value=1");
		}
	}

	BYTE GraphController::ReadAddress()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadAddress, value=%d", m_currAddress);
		return (BYTE)m_currAddress;
	}

	void GraphController::WriteAddress(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteAddress, reg=%d", value);
		value &= 15;
		m_currAddress = (value > (int)GraphControllerAddress::_GRAPH_MAX_REG) ?
			GraphControllerAddress::GRAPH_INVALID_REG :
			(GraphControllerAddress)value;
	}

	BYTE GraphController::ReadValue()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadValue, reg=%d", m_currAddress);
		switch (m_currAddress)
		{
		case GraphControllerAddress::GRAPH_SET_RESET:
			return m_data.setReset;
		case GraphControllerAddress::GRAPH_ENABLE_SET_RESET:
			return m_data.enableSetReset;
		case GraphControllerAddress::GRAPH_COLOR_COMPARE:
			return m_data.colorCompare;
		case GraphControllerAddress::GRAPH_DATA_ROTATE:
			return m_data.rotateCount | (((BYTE)m_data.aluFunction) << 3);
		case GraphControllerAddress::GRAPH_READ_MAP_SELECT:
			return m_data.readPlaneSelect;
		case GraphControllerAddress::GRAPH_MODE:
			return (m_data.writeMode << 0) |
				(m_data.readModeCompare << 3) |
				(m_data.oddEven << 4) |
				(m_data.shiftRegister << 5) |
				(m_data.color256 << 6);
		case GraphControllerAddress::GRAPH_MISC:
			return (m_data.graphics << 0) |
				(m_data.chainOddEven << 1) |
				(((BYTE)m_data.memoryMap) << 2);
		case GraphControllerAddress::GRAPH_COLOR_DONT_CARE:
			return m_data.colorDontCare;
		case GraphControllerAddress::GRAPH_BIT_MASK:
			return m_data.bitMask;
		default:
			LogPrintf(Logger::LOG_WARNING, "WriteValue, Invalid register");
			return 0xFF;
		}
	}

	void GraphController::WriteValue(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteValue, value=%02Xh", value);

		bool oldVal;

		switch (m_currAddress)
		{
		case GraphControllerAddress::GRAPH_SET_RESET:
			m_data.setReset = value & 15;
			LogPrintf(Logger::LOG_INFO, "WriteValue, Set/Reset %d", m_data.setReset);
			break;
		case GraphControllerAddress::GRAPH_ENABLE_SET_RESET:
			m_data.enableSetReset = value & 15;
			LogPrintf(Logger::LOG_INFO, "WriteValue, Enable Set/Reset %d", m_data.enableSetReset);
			break;
		case GraphControllerAddress::GRAPH_COLOR_COMPARE:
			m_data.colorCompare = value & 15;
			LogPrintf(Logger::LOG_INFO, "WriteValue, Color Compare %d", m_data.colorCompare);
			break;
		case GraphControllerAddress::GRAPH_DATA_ROTATE:
			LogPrintf(Logger::LOG_DEBUG, "WriteValue, Data Rotate %d", value);
			m_data.rotateCount = value & 7;
			m_data.aluFunction = (ALUFunction)((value >> 3) & 3);
			LogPrintf(Logger::LOG_INFO, "WriteValue, Data Rotate Count = %d, Function = %s",
				m_data.rotateCount,
				graph_vga::ALUFunctionStr[(int)m_data.aluFunction]);
			break;
		case GraphControllerAddress::GRAPH_READ_MAP_SELECT:
			LogPrintf(Logger::LOG_INFO, "WriteValue, Read Map Select %d", value);
			m_data.readPlaneSelect = value & 3;
			break;
		case GraphControllerAddress::GRAPH_MODE:
			LogPrintf(Logger::LOG_DEBUG, "WriteValue, Mode %d", value);

			m_data.writeMode = value & 3;
			LogPrintf(Logger::LOG_INFO, "WriteValue, WriteMode[%d]", m_data.writeMode);

			m_data.readModeCompare = GetBit(value, 3);
			LogPrintf(Logger::LOG_INFO, "WriteValue, ReadMode[%s]", m_data.readModeCompare ? "COMPARE" : "NORMAL");

			m_data.oddEven = GetBit(value, 4);
			// Same as MemoryMode.oddEven
			LogPrintf(Logger::LOG_INFO, "WriteValue, Odd/Even addressing mode[%d]", m_data.oddEven);

			m_data.shiftRegister = GetBit(value, 5);
			LogPrintf(Logger::LOG_INFO, "WriteValue, Shift register mode[%d]", m_data.shiftRegister);

			oldVal = m_data.color256;
			m_data.color256 = GetBit(value, 6);
			LogPrintf(Logger::LOG_INFO, "WriteValue, 256 color mode [%d]", m_data.color256);
			if (oldVal != m_data.color256)
			{
				FireChangeMode();
			}
			break;
		case GraphControllerAddress::GRAPH_MISC:
			LogPrintf(Logger::LOG_DEBUG, "WriteValue, Miscellaneous %d", value);
			m_data.graphics = GetBit(value, 0);
			LogPrintf(Logger::LOG_INFO, "WriteValue, Graphics[%d]", m_data.graphics);
			m_data.chainOddEven = GetBit(value, 1);
			LogPrintf(Logger::LOG_INFO, "WriteValue, Chain Odd/Even[%d]", m_data.chainOddEven);
			m_data.memoryMap = (MemoryMap)((value >> 2) & 3);
			LogPrintf(Logger::LOG_INFO, "WriteValue, Memory Map[%x]", m_data.memoryMap);
			MapMemory();
			FireChangeMode();
			break;
		case GraphControllerAddress::GRAPH_COLOR_DONT_CARE:
			m_data.colorDontCare = value & 15;
			LogPrintf(Logger::LOG_INFO, "WriteValue, Color Don't Care[%x]", value);
			break;
		case GraphControllerAddress::GRAPH_BIT_MASK:
			m_data.bitMask = value;
			LogPrintf(Logger::LOG_DEBUG, "WriteValue, Bit Mask[%02x]", m_data.bitMask);
			break;
		default:
			LogPrintf(Logger::LOG_WARNING, "WriteValue, Invalid register");
		}
	}

	void GraphController::MapMemory()
	{
		if (!m_vgaRAM)
			return;

		m_memory->Free(m_vgaRAM);

		switch (m_data.memoryMap)
		{
		case MemoryMap::A000_128K:
			LogPrintf(Logger::LOG_INFO, "MapMemory: [A000][128K]");
			m_memory->Allocate(m_vgaRAM, 0xA0000, 0x20000);
			break;
		case MemoryMap::A000_64K:
			LogPrintf(Logger::LOG_INFO, "MapMemory: [A000][64K]");
			m_memory->Allocate(m_vgaRAM, 0xA0000, 0x10000);
			break;
		case MemoryMap::B000_32K:
			LogPrintf(Logger::LOG_INFO, "MapMemory: [B000][32K]");
			m_memory->Allocate(m_vgaRAM, 0xB0000, 0x8000);
			break;
		case MemoryMap::B800_32K:
			LogPrintf(Logger::LOG_INFO, "MapMemory: [B800][32K]");
			m_memory->Allocate(m_vgaRAM, 0xB8000, 0x8000);
			break;
		}
	}

	void GraphController::Serialize(json& to)
	{
		to["currAddress"] = m_currAddress;

		to["setReset"] = m_data.setReset;
		to["enableSetReset"] = m_data.enableSetReset;
		to["colorCompare"] = m_data.colorCompare;
		to["rotateCount"] = m_data.rotateCount;
		to["aluFunction"] = m_data.aluFunction;
		to["readPlaneSelect"] = m_data.readPlaneSelect;
		to["writeMode"] = m_data.writeMode;
		to["readModeCompare"] = m_data.readModeCompare;
		to["oddEven"] = m_data.oddEven;
		to["shiftRegister"] = m_data.shiftRegister;
		to["color256"] = m_data.color256;		
		to["graphics"] = m_data.graphics;
		to["chainOddEven"] = m_data.chainOddEven;
		to["memoryMap"] = m_data.memoryMap;
		to["colorDontCare"] = m_data.colorDontCare;
		to["bitMask"] = m_data.bitMask;
	}

	void GraphController::Deserialize(const json& from)
	{
		m_currAddress = from["currAddress"];

		m_data.setReset = from["setReset"];
		m_data.enableSetReset = from["enableSetReset"];
		m_data.colorCompare = from["colorCompare"];
		m_data.rotateCount = from["rotateCount"];
		m_data.aluFunction = from["aluFunction"];
		m_data.readPlaneSelect = from["readPlaneSelect"];
		m_data.writeMode = from["writeMode"];
		m_data.readModeCompare = from["readModeCompare"];
		m_data.oddEven = from["oddEven"];
		m_data.shiftRegister = from["shiftRegister"];
		m_data.color256 = from["color256"];
		m_data.graphics = from["graphics"];
		m_data.chainOddEven = from["chainOddEven"];
		m_data.memoryMap = from["memoryMap"];
		m_data.colorDontCare = from["colorDontCare"];
		m_data.bitMask = from["bitMask"];

		MapMemory();
	}
}