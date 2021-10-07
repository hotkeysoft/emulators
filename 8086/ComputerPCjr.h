#pragma once

#include "CPU8086.h"
#include "Memory.h"
#include "Device8254.h"
#include "Device8255PCjr.h"
#include "Device8259.h"
#include "DevicePCSpeaker.h"
#include "VideoPCjr.h"

namespace emul
{
	class ComputerPCjr : public CPU8086
	{
	public:
		ComputerPCjr();

		void Init();

		virtual bool Step() override;

		bool LoadBinary(const char* file, ADDRESS baseAddress) { return m_memory.LoadBinary(file, baseAddress); }
		
		void InputKey(BYTE ch) { m_keyBuf[m_keyBufWrite++] = ch; }

		Memory& GetMemory() { return m_memory; }

		size_t GetTicks() { return m_ticks; }

	protected:
		size_t m_ticks = 0;

		Memory m_memory;

		// TODO: Should be dynamic
		emul::MemoryBlock m_base64K;
		emul::MemoryBlock m_biosF000;
		emul::MemoryBlock m_biosF800;

		MemoryMap m_map;

		pit::Device8254 m_pit;
		pic::Device8259 m_pic;
		ppi::Device8255PCjr m_ppi;
		//fdc::DeviceFloppy m_floppy;

		video::VideoPCjr m_video;

		beeper::DevicePCSpeaker m_pcSpeaker;

		// Keyboard buffer
		BYTE m_keyBuf[256];
		BYTE m_keyBufRead = 0;
		BYTE m_keyBufWrite = 0;
	};
}
