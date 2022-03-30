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
	enum class DISPLAY
	{
		CGA = 0,
		MDA = 1,

		OTHER = 0
	};

	enum class Command
	{
		CMD_NONE = 0,

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

		virtual bool IsSoundON() override { return m_portB.speakerDataEnabled; }
		virtual void SetCurrentKeyCode(BYTE keyCode) override {}

		void Tick();

		void SetCPU(emul::CPU8086* cpu);

		// Timer 1 output, indicates ram refresh
		void SetRefresh(bool refreshBit);

		bool GetTimer2Gate() const { return m_portB.timer2Gate; }
		void SetTimer2Output(bool value) { m_portB.timer2Output = value; }

		// Configuration Switches
		//
		// Connected to the lock on the front of the panel.
		void SetKeyLock(bool lock) { m_switches.keyboardLock = lock; }
		//
		// Manufacturing mode jumper, makes POST loop forever
		void SetPOSTLoop(bool en) { m_switches.manufacturing = en; }
		//
		// Tells the motherboard what kind of display to initialize (CGA | MDA)
		// Switch is don't care for cards with onboard ROM that do their own setup (EGA | VGA)
		void SetDisplayConfig(DISPLAY disp) { m_switches.display = disp; }

	protected:
		emul::CPU80286* m_cpu = nullptr;

		BYTE m_inputBuffer;
		BYTE m_outputBuffer;

		void WriteInputBuffer(BYTE value);
		void WriteOutputBuffer(BYTE value);
		BYTE ReadInputBuffer();
		BYTE ReadOutputBuffer();

		Command m_activeCommand = Command::CMD_NONE;

		// Configuration Swiches on motherboard
		struct Switches
		{
			bool keyboardLock = false; // Bit 7
			DISPLAY display = DISPLAY::OTHER; // Bit 6
			bool manufacturing = false; // Bit 5
			bool ramsel = false; // Bit 4 256/512k decoding on main board
			// Bit 3-0 unused
		} m_switches;
		void ReadConfigurationSwitches();

		struct Status
		{
			bool parityError = false;
			bool receiveTimeout = false;
			bool transmitTimeout = false;
			bool inhibit = false;
			enum class Mode { DATA = 0, COMMAND } mode = Mode::DATA;
			bool systemFlag = false;
			bool inputBufferFull = false;
			bool outputBufferFull = false;

			void Reset() { memset(this, 0, sizeof(Status)); }
		} m_status;

		BYTE ReadStatus();
		void WriteCommand(BYTE value);

		struct PortB
		{
			bool parityCheckOccurred = false;  // R
			bool channelCheckOccurred = false; // R
			bool timer2Output = false; // R
			BYTE refresh = 0; // R
			bool channelCheckEnabled = false; // R/W
			bool parityCheckEnabled = false; // R/W
			bool speakerDataEnabled = false; // R/W
			bool timer2Gate = false; // R/W
		} m_portB;
		BYTE ReadPortB();
		void WritePortB(BYTE value);

		struct CommandByte
		{
			bool pcCompatibilityMode = false;
			bool pcComputerMode = false;
			bool disableKeyboard = false;
			bool inhibitOverride = false;
			bool outputBufferFullInterrupt = false;
		} m_commandByte;
		void ReadCommandByte();
		void WriteCommandByte();

		size_t m_commandDelay = 0;
		void StartSelfTest();
		void EndSelfTest();
	};
}
