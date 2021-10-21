#pragma once

#include "CPU8086.h"
#include "Memory.h"
#include "DeviceFloppy.h"
#include "InputEvents.h"

namespace emul
{
	class Computer : public CPU8086
	{
	public:
		virtual ~Computer() {}

		virtual void Init() = 0;

		bool LoadBinary(const char* file, ADDRESS baseAddress) { return m_memory.LoadBinary(file, baseAddress); }
		
		Memory& GetMemory() { return m_memory; }
		virtual fdc::DeviceFloppy& GetFloppy() = 0;
		virtual kbd::DeviceKeyboard& GetKeyboard() = 0;

		virtual void Reboot(bool hard = false);
		void SetTurbo(bool turbo) { m_turbo = turbo; }

	protected:
		Computer(Memory& memory, MemoryMap& mmap);

		Memory m_memory;
		MemoryMap m_map;

		bool m_turbo = false;
	};
}
