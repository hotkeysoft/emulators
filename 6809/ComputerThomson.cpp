#include "stdafx.h"

#include "ComputerThomson.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU6809.h"
#include <Video/VideoNull.h>

using cfg::CONFIG;

using ScreenRAM = pia::Device6520MO5_PIA::ScreenRAM;

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
		GetVideo().Reset();
		m_pia.Reset();
		m_keyboard.Reset();
	}

	void ComputerThomson::Init(WORD baseRAM)
	{
		ComputerBase::Init(emul::CPUID_6809, baseRAM);

		InitROM();
		InitRAM();
		InitIO();
		InitInputs(1000000, 100000);
		GetInputs().InitKeyboard(&m_keyboard);

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

		MapScreenMem(false);
	}

	void ComputerThomson::InitIO()
	{
		m_pia.EnableLog(CONFIG().GetLogLevel("pia"));
		m_pia.Init(&m_keyboard);

		m_ioA7C0.AddDevice(m_pia, 0, 0b111100);
		m_memory.Allocate(&m_ioA7C0, 0xA7C0);
	}

	void ComputerThomson::MapScreenMem(bool forme)
	{
		if (forme != m_forme)
		{
			m_memory.Allocate(m_forme ? &m_pixelRAM : &m_attributeRAM, 0);
			m_forme = forme;
		}
	}

	//void ComputerThomson::WritePortA(BYTE value)
	//{
	//	m_portA = value;

	//	//bit0: / FORME : screen ram mapping(pixel or palette data) for 0x0000..0x1FFF
	//	bool forme = GetBit(value, 0);

	//	if (forme != m_forme)
	//	{
	//		m_forme = forme;
	//		MapScreenMem();
	//	}

	//	//bit1 : RT: red value for border
	//	//bit2 : VT: green value for border
	//	//bit3 : BT: blue value for border
	//	//bit4 : PT: 'pastel' value for border
	//	BYTE border = (value >> 1) & 0b1111;

	//	//bit5 : Lightpen interrupt(input)

	//	//bit6 : Cassette OUT
	//	bool cassette = GetBit(value, 6);

	//	//bit7 : Cassette input.when idle, the tape drive sets this to a logic 1, and the code checks for it.

	//	LogPrintf(LOG_DEBUG, "WritePortA: [%cFORME] [BORDER$%x] [%cCASSETTE]",
	//		forme ? ' ' : '/',
	//		border,
	//		cassette ? ' ' : '/');
	//}

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

		sprintf(buf, "dump/RAM_USER_%llu.bin", time(nullptr));
		fprintf(stderr, "Dump USER RAM to %s\n", buf);
		m_userRAM.Dump(buf);

		sprintf(buf, "dump/RAM_PIXEL_%llu.bin", time(nullptr));
		fprintf(stderr, "Dump PIXEL RAM to %s\n", buf);
		m_pixelRAM.Dump(buf);

		sprintf(buf, "dump/RAM_ATTR_%llu.bin", time(nullptr));
		fprintf(stderr, "Dump ATTR RAM to %s\n", buf);
		m_attributeRAM.Dump(buf);
	}

	void ComputerThomson::DrawScreen()
	{
		m_video->BeginFrame();
		const BYTE* pixels = m_pixelRAM.getPtr();
		for (int y = 0; y < 200; ++y)
		{
			for (int x = 0; x < 40; ++x)
			{
				for (int i = 0; i < 8; ++i)
				{
					m_video->DrawPixel(GetBit(*pixels, 7 - i) ? 0xFFFFFFFF : 0);
				}

				pixels++;
			}
			m_video->NewLine();
		}

		m_video->RenderFrame();
	}

	bool ComputerThomson::Step()
	{
		if (!ComputerBase::Step())
		{
			DumpRAM();
			DrawScreen();

			return false;
		}

		uint32_t cpuTicks = GetCPU().GetInstructionTicks();

		// TODO: Lame
		MapScreenMem(m_pia.GetScreenMapping() == ScreenRAM::PIXEL);

		for (uint32_t i = 0; i < cpuTicks; ++i)
		{
			++g_ticks;

			if ((g_ticks % 100000) == 0)
			{
				DrawScreen();
			}

			m_video->Tick();

			GetInputs().Tick();
			if (GetInputs().IsQuit())
			{
				return false;
			}
		}

		return true;
	}

	void ComputerThomson::Serialize(json& to)
	{
		ComputerBase::Serialize(to);
		//to["model"] = ModelToString(m_model);

		m_pia.Serialize(to["pia"]);

	}

	void ComputerThomson::Deserialize(const json& from)
	{
		ComputerBase::Deserialize(from);
		//std::string modelStr = from["model"];
		//Model model = StringToModel(modelStr.c_str());
		//if ((m_model == Model::UNKNOWN) || (model != m_model))
		//{
		//	throw SerializableException("Computer: Model is not compatible", SerializationError::COMPAT);
		//}

		m_pia.Deserialize(from["pia"]);
	}
}
