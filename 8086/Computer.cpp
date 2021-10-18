#include "Common.h"
#include "Computer.h"
#include "DeviceKeyboard.h"

namespace emul
{
	Computer::Computer(Memory& memory, MemoryMap& mmap) :
		Logger("PC"),
		CPU8086(m_memory, m_map),
		m_memory(emul::CPU8086_ADDRESS_BITS)
	{
	}

	void Computer::Reboot(bool hard)
	{
		if (hard)
		{
			Reset();
		}
		else
		{
			GetKeyboard().InputKey(0x1D); // CTRL
			GetKeyboard().InputKey(0x38); // ALT
			GetKeyboard().InputKey(0x53); // DELETE
		}
	}
}
