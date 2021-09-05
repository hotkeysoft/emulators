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

		void SendKey(char ch);

	protected:
		void WriteValueHex(BYTE value, const CPUInfo::Coord& coord, WORD attr = 15);
		void WriteValueHex(WORD value, const CPUInfo::Coord& coord, WORD attr = 15);

		void UpdateRegisters();
		void UpdateRAM();
		void UpdateCode();

		CPU8086* m_cpu;
		Memory* m_memory;
		Console& m_console;
	};
}
