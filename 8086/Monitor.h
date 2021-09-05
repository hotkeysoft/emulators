#pragma once

#include "Console.h"
#include "Memory.h"
#include "CPU8086.h"
#include "CPUInfo.h"

namespace emul
{
	class Monitor
	{
	public:
		Monitor(Console& console);

		Monitor() = delete;
		Monitor(const Monitor&) = delete;
		Monitor& operator=(const Monitor&) = delete;
		Monitor(Monitor&&) = delete;
		Monitor& operator=(Monitor&&) = delete;

		void Init(CPU8086& cpu, Memory& memory);

		void Show();
		void Update();
		void ToggleRunMode();
		void UpdateRunMode();
		void ToggleRAMMode();
		void UpdateRAMMode();
		void Step();

		void SendKey(char ch);

	protected:
		void WriteValueHex(BYTE value, const CPUInfo::Coord& coord, WORD attr = 15);
		void WriteValueHex(WORD value, const CPUInfo::Coord& coord, WORD attr = 15);

		void UpdateRegisters();
		void UpdateFlags();
		void UpdateRAM();
		void UpdateCode();

		enum class RUNMode { STEP, RUN };
		RUNMode m_runMode = RUNMode::STEP;

		enum class RAMMode { DSSI, ESDI, STACK, CUSTOM };
		RAMMode m_ramMode = RAMMode::DSSI;

		CPU8086* m_cpu = nullptr;
		Memory* m_memory = nullptr;
		Console& m_console;
	};
}
