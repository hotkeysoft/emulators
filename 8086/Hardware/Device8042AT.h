#pragma once

#include "../Serializable.h"
#include "DevicePPI.h"

// Microcontroller that manages communication with the keyboard (AT version)
// 
// It also partially replicates the 8255 ports on the XT
// Finally it allows some low level hardware manipulations
// such as handling the A20 line or resetting the CPU

namespace emul
{
	class CPU8086;
	class CPU80286;
}

namespace ppi
{
	class Device8042AT : public ppi::DevicePPI
	{
	public:
		Device8042AT(WORD baseAddress);

		Device8042AT() = delete;
		Device8042AT(const Device8042AT&) = delete;
		Device8042AT& operator=(const Device8042AT&) = delete;
		Device8042AT(Device8042AT&&) = delete;
		Device8042AT& operator=(Device8042AT&&) = delete;

		virtual void Init() override;

		virtual bool IsSoundON() override { return false; }
		virtual void SetCurrentKeyCode(BYTE keyCode) override {}

		void SetCPU(emul::CPU8086* cpu);

		// Timer 1 output, indicates ram refresh
		void SetRefresh(bool refreshBit) { m_portB.refresh = refreshBit; }

	protected:
		emul::CPU80286* m_cpu = nullptr;

		BYTE ReadBuffer();
		void WriteBuffer(BYTE value);

		BYTE ReadStatus();
		void WriteCommand(BYTE value);

		struct PortB
		{
			bool refresh = false;
		} m_portB;
		BYTE ReadPortB();
		void WritePortB(BYTE value);
	};
}
