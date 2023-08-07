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

		MapScreenMem();
	}

	void ComputerThomson::InitIO()
	{
		m_memory.Allocate(&m_ioA7C0, 0xA7C0);

		// TODO: TEMP
		IOConnector::Connect(0, static_cast<IOConnector::READFunction>(&ComputerThomson::ReadPortA));
		IOConnector::Connect(0, static_cast<IOConnector::WRITEFunction>(&ComputerThomson::WritePortA));

		IOConnector::Connect(1, static_cast<IOConnector::READFunction>(&ComputerThomson::ReadPortB));
		IOConnector::Connect(1, static_cast<IOConnector::WRITEFunction>(&ComputerThomson::WritePortB));

		IOConnector::Connect(2, static_cast<IOConnector::READFunction>(&ComputerThomson::ReadControlA));
		IOConnector::Connect(2, static_cast<IOConnector::WRITEFunction>(&ComputerThomson::WriteControlA));

		IOConnector::Connect(3, static_cast<IOConnector::READFunction>(&ComputerThomson::ReadControlB));
		IOConnector::Connect(3, static_cast<IOConnector::WRITEFunction>(&ComputerThomson::WriteControlB));


		for (int i = 4; i < 64; ++i)
		{
			IOConnector::Connect(i, static_cast<IOConnector::READFunction>(&ComputerThomson::ReadIO));
			IOConnector::Connect(i, static_cast<IOConnector::WRITEFunction>(&ComputerThomson::WriteIO));
		}

		m_ioA7C0.AddDevice(*this, 0);
	}

	void ComputerThomson::MapScreenMem()
	{
		m_memory.Allocate(m_forme ? &m_pixelRAM : &m_attributeRAM, 0);
	}

	BYTE ComputerThomson::ReadIO()
	{
		LogPrintf(LOG_TRACE, "ReadIO()");
		return 0xFF;
	}

	void ComputerThomson::WriteIO(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteIO(), value=%02X", value);
	}

	BYTE ComputerThomson::ReadPortA()
	{
		LogPrintf(LOG_INFO, "ReadPortA()");

		return m_portA | 0b10100000;
	}
	void ComputerThomson::WritePortA(BYTE value)
	{
		m_portA = value;

		//bit0: / FORME : screen ram mapping(pixel or palette data) for 0x0000..0x1FFF
		bool forme = GetBit(value, 0);

		if (forme != m_forme)
		{
			m_forme = forme;
			MapScreenMem();
		}

		//bit1 : RT: red value for border
		//bit2 : VT: green value for border
		//bit3 : BT: blue value for border
		//bit4 : PT: 'pastel' value for border
		BYTE border = (value >> 1) & 0b1111;

		//bit5 : Lightpen interrupt(input)

		//bit6 : Cassette OUT
		bool cassette = GetBit(value, 6);

		//bit7 : Cassette input.when idle, the tape drive sets this to a logic 1, and the code checks for it.

		LogPrintf(LOG_INFO, "WritePortA: [%cFORME] [BORDER$%x] [%cCASSETTE]",
			forme ? ' ' : '/',
			border,
			cassette ? ' ' : '/');
	}

	BYTE ComputerThomson::ReadPortB()
	{
		LogPrintf(LOG_INFO, "ReadPortB()");
		return 0xFF;
	}
	void ComputerThomson::WritePortB(BYTE value)
	{
		LogPrintf(LOG_INFO, "WritePortB(), value=%02X", value);
	}

	BYTE ComputerThomson::ReadControlA()
	{
		LogPrintf(LOG_INFO, "ReadControlA()");
		return 0xFF;
	}
	void ComputerThomson::WriteControlA(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteControlA(), value=%02X", value);
	}

	BYTE ComputerThomson::ReadControlB()
	{
		LogPrintf(LOG_INFO, "ReadControlB()");
		return 0xFF;
	}
	void ComputerThomson::WriteControlB(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteControlB(), value=%02X", value);
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
		m_userRAM.Dump(buf);

		sprintf(buf, "dump/RAM_PIXEL_%zu.bin", time(nullptr));
		fprintf(stderr, "Dump PIXEL RAM to %s\n", buf);
		m_pixelRAM.Dump(buf);

		sprintf(buf, "dump/RAM_ATTR_%zu.bin", time(nullptr));
		fprintf(stderr, "Dump ATTR RAM to %s\n", buf);
		m_attributeRAM.Dump(buf);


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
