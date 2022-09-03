#pragma once
#include <CPU/CPUCommon.h>
#include <Serializable.h>
#include <CPU/PortConnector.h>
#include "VideoEvents.h"

namespace emul
{
	class Memory;
	class MemoryBlock;
}

namespace graph_ega
{
	enum class ALUFunction { NONE = 0, AND, OR, XOR };
	const char* const ALUFunctionStr[] = { "NONE", "AND", "OR", "XOR" };

	enum class MemoryMap { A000_128K = 0, A000_64K, B000_32K, B800_32K };

	struct GraphControllerData
	{
		// Set/Reset Register (0)
		BYTE setReset = 0;
		// Enable Set/Reset Register (1)
		BYTE enableSetReset = 0;

		// Color Compare Register (2)
		BYTE colorCompare = 0;

		// Data Rotate Register (3)
		BYTE rotateCount = 0;
		ALUFunction aluFunction = ALUFunction::NONE;

		// Read Map Select Register (4)
		BYTE readPlaneSelect = 0;

		// Mode Register (5)
		BYTE writeMode = 0;
		bool readModeCompare = false;
		bool oddEven = false;
		bool shiftRegister = false;

		// Miscellaneous Register (6)
		bool graphics = false;
		bool chainOddEven = false;
		MemoryMap memoryMap = MemoryMap::A000_128K;

		// Color Compare Register (7)
		BYTE colorDontCare = 0;

		// Bit Mask Register
		BYTE bitMask = 0;

		void Reset();
	};

	class GraphController : public emul::PortConnector, public emul::Serializable, public vid_events::EventSource
	{
	public:
		GraphController(WORD baseAddress);

		GraphController() = delete;
		GraphController(const GraphController&) = delete;
		GraphController& operator=(const GraphController&) = delete;
		GraphController(GraphController&&) = delete;
		GraphController& operator=(GraphController&&) = delete;

		virtual void Init(emul::Memory* memory, emul::MemoryBlock* egaRAM);
		virtual void Reset();

		const GraphControllerData& GetData() const { return m_data; }

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		const WORD m_baseAddress;

		enum class GraphControllerAddress
		{
			GRAPH_SET_RESET = 0,
			GRAPH_ENABLE_SET_RESET = 1,
			GRAPH_COLOR_COMPARE = 2,
			GRAPH_DATA_ROTATE = 3,
			GRAPH_READ_MAP_SELECT = 4,
			GRAPH_MODE = 5,
			GRAPH_MISC = 6,
			GRAPH_COLOR_DONT_CARE = 7,
			GRAPH_BIT_MASK = 8,

			_GRAPH_MAX_REG = GRAPH_BIT_MASK,
			GRAPH_INVALID_REG = 0xFF
		} m_currAddress = GraphControllerAddress::GRAPH_INVALID_REG;

		void WritePosition1(BYTE value);
		void WritePosition2(BYTE value);
		void WriteAddress(BYTE value);
		void WriteValue(BYTE value);

		void MapMemory();

		GraphControllerData m_data;

	private:
		emul::Memory* m_memory = nullptr;
		emul::MemoryBlock* m_egaRAM = nullptr;
	};
}
