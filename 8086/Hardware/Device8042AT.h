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
	enum class Command
	{
		CMD_READ_COMMAND_BYTE = 0x20,
		CMD_WRITE_COMMAND_BYTE = 0x60,
		CMD_SELF_TEST = 0xAA,
		CMD_INTERFACE_TEST = 0xAB,
		CMD_DIAGNOSTIC_DUMP = 0xAC,
		CMD_DISABLE_KEYBOARD = 0xAD,
		CMD_ENABLE_KEYBOARD = 0xAE,
		CMD_READ_INPUT_PORT = 0xC0,
		CMD_READ_OUTPUT_PORT = 0xD0,
		CMD_WRITE_OUTPUT_PORT = 0xD1,
		CMD_READ_TEST_INPUTS = 0xE0,

		// Low nibble is mask of bits to be pulsed
		CMD_PULSE_OUTPUT_PORT_MIN = 0xF0,
		CMD_PULSE_OUTPUT_PORT_MAX = 0xFF,
	};

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

		void Tick();

		void SetCPU(emul::CPU8086* cpu);

		// Timer 1 output, indicates ram refresh
		void SetRefresh(bool refreshBit);

	protected:
		emul::CPU80286* m_cpu = nullptr;

		BYTE m_inputBuffer;
		BYTE m_outputBuffer;

		void WriteInputBuffer(BYTE value);
		void WriteOutputBuffer(BYTE value);
		BYTE ReadInputBuffer();
		BYTE ReadOutputBuffer();

		struct Status
		{
			bool parityError = false;
			bool receiveTimeout = false;
			bool transmitTimeout = false;
			bool inhibit = false;
			enum class Mode { DATA = 0, COMMAND } mode = Mode::DATA;
			bool selfTestOK = false;
			bool inputBufferFull = false;
			bool outputBufferFull = false;

			void Reset() { memset(this, 0, sizeof(Status)); }
		} m_status;

		BYTE ReadStatus();
		void WriteCommand(BYTE value);

		struct PortB
		{
			BYTE refresh = 0;

		} m_portB;
		BYTE ReadPortB();
		void WritePortB(BYTE value);

		size_t m_commandDelay = 0;
		bool m_selfTest = false;
		void StartSelfTest();
		void EndSelfTest();
	};
}
