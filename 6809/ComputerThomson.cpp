#include "stdafx.h"

#include "ComputerThomson.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU6809.h"
#include <Video/VideoNull.h>

using cfg::CONFIG;

namespace emul
{
	ComputerThomson::ComputerThomson() :
		Logger("Thomson"),
		ComputerBase(m_memory, 64),
		IOConnector(63), // TODO: Temp, decode whole block
		m_pixelRAM("RAM_PIXEL", 0x2000, emul::MemoryType::RAM),
		m_attributeRAM("RAM_ATTR", 0x2000, emul::MemoryType::RAM),
		m_userRAM("RAM_USER", 0x8000, emul::MemoryType::RAM),
		m_osROM("ROM_OS", 0x1000, emul::MemoryType::ROM),
		m_basicROM("ROM_BASIC", 0x3000, emul::MemoryType::ROM),
		m_ioA7C0("IO", 0x40)
	{
	}

	void ComputerThomson::Reset()
	{
		ComputerBase::Reset();
	}

	void ComputerThomson::Init(WORD baseRAM)
	{
		ComputerBase::Init(emul::CPUID_6809, baseRAM);

		InitROM();
		InitRAM();
		InitIO();
		InitInputs(1000000, 100000);
		InitVideo();
	}

	void ComputerThomson::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_6809) m_cpu = new CPU6809(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void ComputerThomson::InitROM()
	{
		m_osROM.LoadFromFile("data/Thomson/MO5/mo5.os.bin");
		m_memory.Allocate(&m_osROM, 0xF000);

		m_basicROM.LoadFromFile("data/Thomson/MO5/mo5.basic.bin");
		m_memory.Allocate(&m_basicROM, 0xC000);
	}

	void ComputerThomson::InitRAM()
	{
		m_userRAM.Clear(0x69);
		m_memory.Allocate(&m_userRAM, 0x2000);

		m_pixelRAM.Clear(0xFA);
		m_attributeRAM.Clear(0xAF);

		m_memory.Allocate(&m_pixelRAM, 0); // Paged in/out with m_attributeRAM
	}

	void ComputerThomson::InitIO()
	{
		m_memory.Allocate(&m_ioA7C0, 0xA7C0);

		// TODO: TEMP
		for (int i = 0; i < 64; ++i)
		{
			IOConnector::Connect(i, static_cast<IOConnector::READFunction>(&ComputerThomson::ReadIO));
			IOConnector::Connect(i, static_cast<IOConnector::WRITEFunction>(&ComputerThomson::WriteIO));
		}

		m_ioA7C0.AddDevice(*this, 0);
	}

	BYTE ComputerThomson::ReadIO()
	{
		LogPrintf(LOG_INFO, "ReadIO()");
		return 0xFF;
	}

	void ComputerThomson::WriteIO(BYTE value)
	{
		GetCurrentPort();
		LogPrintf(LOG_INFO, "WriteIO(), value=%02X");
	}

	void ComputerThomson::InitVideo()
	{
		// Dummy video card
		video::VideoNull* video = new video::VideoNull();
		video->EnableLog(CONFIG().GetLogLevel("video"));
		video->Init(&m_memory, nullptr);
		m_video = video;
	}

	void ComputerThomson::DumpRAM()
	{
		char buf[128];
		sprintf(buf, "dump/RAM_USER_%zu.bin", time(nullptr));
		fprintf(stderr, "Dump USER RAM to %s\n", buf);
		m_memory.Dump(0x2000, 0x8000, buf);

		sprintf(buf, "dump/RAM_PIXEL_%zu.bin", time(nullptr));
		fprintf(stderr, "Dump PIXEL RAM to %s\n", buf);
		m_memory.Dump(0, 0x2000, buf);
	}

	bool ComputerThomson::Step()
	{
		if (!ComputerBase::Step())
		{
			DumpRAM();
			return false;
		}

		static uint32_t cpuTicks = 0;
		cpuTicks += GetCPU().GetInstructionTicks();

		++g_ticks;

		m_video->Tick();

		GetInputs().Tick();
		if (GetInputs().IsQuit())
		{
			return false;
		}

		return true;
	}
}
