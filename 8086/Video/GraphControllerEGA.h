#pragma once

#include "../Common.h"
#include "../Serializable.h"

namespace graph_ega
{
	enum class RotateFunction { NONE = 0, AND, OR, XOR };
	enum class MemoryMap { A000_128K = 0, A000_64K, B000_32K, B800_32K };

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
	};

	class GraphController : public emul::Serializable
	{
	public:
		GraphControllerAddress currRegister = GraphControllerAddress::GRAPH_INVALID_REG;

		// Set/Reset Register (0)
		BYTE setReset = 0;
		// Enable Set/Reset Register (1)
		BYTE enableSetReset = 0;

		// Color Compare Register (2)
		BYTE colorCompare = 0;

		// Data Rotate Register (3)
		BYTE rotateCount = 0;
		RotateFunction rotateFunction = RotateFunction::NONE;

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

		// emul::Serializable
		virtual void Serialize(json& to) override
		{
			to["currRegister"] = currRegister;
			to["setReset"] = setReset;
			to["enableSetReset"] = enableSetReset;
			to["colorCompare"] = colorCompare;
			to["rotateCount"] = rotateCount;
			to["rotateFunction"] = rotateFunction;
			to["readPlaneSelect"] = readPlaneSelect;
			to["writeMode"] = writeMode;
			to["readModeCompare"] = readModeCompare;
			to["oddEven"] = oddEven;
			to["shiftRegister"] = shiftRegister;
			to["graphics"] = graphics;
			to["chainOddEven"] = chainOddEven;
			to["memoryMap"] = memoryMap;
			to["colorDontCare"] = colorDontCare;
			to["bitMask"] = bitMask;
		}

		virtual void Deserialize(json& from) override
		{
			currRegister = from["currRegister"];
			setReset = from["setReset"];
			enableSetReset = from["enableSetReset"];
			colorCompare = from["colorCompare"];
			rotateCount = from["rotateCount"];
			rotateFunction = from["rotateFunction"];
			readPlaneSelect = from["readPlaneSelect"];
			writeMode = from["writeMode"];
			readModeCompare = from["readModeCompare"];
			oddEven = from["oddEven"];
			shiftRegister = from["shiftRegister"];
			graphics = from["graphics"];
			chainOddEven = from["chainOddEven"];
			memoryMap = from["memoryMap"];
			colorDontCare = from["colorDontCare"];
			bitMask = from["bitMask"];
		}
	};
}
