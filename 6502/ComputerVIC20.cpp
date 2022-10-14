#include "stdafx.h"

#include "ComputerVIC20.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU6502.h"
#include <Sound/Sound.h>
#include "IO/DeviceKeyboardPET2001.h"
#include "Video/VideoVIC.h"

using cfg::CONFIG;
using sound::SOUND;
using tape::TapeDeck;

namespace emul
{
	const size_t MAIN_CLK = 16000000; // 16 MHz Main crystal
	const size_t PIXEL_CLK = MAIN_CLK / 2;
	const size_t CPU_CLK = PIXEL_CLK / 8;

	// Poll each frame
	const size_t SCAN_RATE = (262 * 64);

	ComputerVIC20::ComputerVIC20() :
		Logger("VIC20"),
		ComputerBase(m_memory, 256),
		m_ramBlock0LOW("RAM-LOW", 0x400),
		m_ramBlock0RAM1("RAM1", 0x400),
		m_ramBlock0RAM2("RAM2", 0x400),
		m_ramBlock0RAM3("RAM3", 0x400),
		m_ramBlock0MAIN("RAM-MAIN", 0x1000),
		m_ramBlock1("RAM-BLK1", 0x2000),
		m_ramBlock2("RAM-BLK2", 0x2000),
		m_ramBlock3("RAM-BLK3", 0x2000),
		m_romCHAR("ROM-CHAR", 0x1000, emul::MemoryType::ROM),
		m_ramCOLOR("RAM-COLOR", 0x400),
		m_ramBlock5("RAM-BLK5", 0x2000),
		m_romBASIC("ROM-BASIC", 0x2000, emul::MemoryType::ROM),
		m_romKERNAL("ROM-KERNAL", 0x2000, emul::MemoryType::ROM),
		m_ioVIC("IOVIC", 0x0100),
		m_ioVIA("IOVIA", 0x0300),
		m_via1("VIA1"),
		m_via2("VIA2"),
		m_tape(CPU_CLK)
	{
	}

	ComputerVIC20::~ComputerVIC20()
	{
		delete m_keyboard;
	}

	void ComputerVIC20::Init(WORD baseRAM)
	{
		ComputerBase::Init(CPUID_6502, baseRAM);

		InitKeyboard();
		InitRAM(baseRAM);
		InitROM();
		InitIO();
		InitVideo();
		InitTape();

		InitInputs(CPU_CLK, SCAN_RATE);
		GetInputs().InitKeyboard(m_keyboard);

		SOUND().SetBaseClock(CPU_CLK);
	}

	void ComputerVIC20::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_6502) m_cpu = new CPU6502(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void ComputerVIC20::InitKeyboard()
	{
		m_keyboard = new kbd::DeviceKeyboardPET2001();
		m_keyboard->EnableLog(CONFIG().GetLogLevel("keyboard"));
	}

	void ComputerVIC20::InitROM()
	{
		m_romCHAR.LoadFromFile("data/VIC20/CHAR-8000.bin");
		m_memory.Allocate(&m_romCHAR, 0x8000);

		m_romBASIC.LoadFromFile("data/VIC20/BASIC-C000.bin");
		m_memory.Allocate(&m_romBASIC, 0xC000);

		m_romKERNAL.LoadFromFile("data/VIC20/KERNAL-E000-ntsc.bin");
		m_memory.Allocate(&m_romKERNAL, 0xE000);
	}

	void ComputerVIC20::InitVideo()
	{
		video::VideoVIC* video = new video::VideoVIC();
		video->EnableLog(CONFIG().GetLogLevel("video"));
		video->Init(&m_memory, nullptr);
		m_video = video;
	}

	void ComputerVIC20::InitIO()
	{
		m_memory.Allocate(&m_ioVIA, 0x9100);

		// VIA1 @ 0x91[10]
		m_via1.EnableLog(CONFIG().GetLogLevel("vic20.via1"));
		m_via1.Init();
		// Incomplete decoding, will also select at 3x, 5x, 7x etc
		m_ioVIA.AddDevice(m_via1, 0x10);

		// VIA2 @ 0x91[20]
		m_via2.EnableLog(CONFIG().GetLogLevel("vic20.via2"));
		m_via2.Init();
		// Incomplete decoding, will also select at 3x, 6x, 7x, Ax, Bx, etc
		m_ioVIA.AddDevice(m_via2, 0x20);
	}

	void ComputerVIC20::InitRAM(emul::WORD baseRAM)
	{
		LogPrintf(LOG_INFO, "Requested base RAM: %dKB", baseRAM);

		if (baseRAM < 5)
		{
			LogPrintf(LOG_WARNING, "Requested base RAM too low (%dKB), using 4KB", baseRAM);
			baseRAM = 5;
		}
		else if (baseRAM > 32)
		{
			baseRAM = 32;
			LogPrintf(LOG_WARNING, "Requested base RAM too low (%dKB), using 32KB", baseRAM);
		}

		// Above 8K, Round to 8k block
		if (baseRAM > 8)
		{
			WORD rounded = (baseRAM & 0xF8);
			if (rounded != baseRAM)
			{
				baseRAM = rounded;
				LogPrintf(LOG_WARNING, "Requested base RAM rounded to (%dKB)", baseRAM);
			}
		}

		switch (baseRAM)
		{
		case 32:
			m_memory.Allocate(&m_ramBlock1, 0x6000);
			[[fallthrough]];
		case 24:
			m_memory.Allocate(&m_ramBlock1, 0x4000);
			[[fallthrough]];
		case 16:
			m_memory.Allocate(&m_ramBlock1, 0x2000);
			[[fallthrough]];
		case 8:
			m_memory.Allocate(&m_ramBlock0RAM3, 0x0C00);
			[[fallthrough]];
		case 7:
			m_memory.Allocate(&m_ramBlock0RAM2, 0x0800);
			[[fallthrough]];
		case 6:
			m_memory.Allocate(&m_ramBlock0RAM1, 0x0400);
			[[fallthrough]];
		default:
			// Base 5K: Always allocated
			m_memory.Allocate(&m_ramBlock0LOW, 0x0000);
			m_memory.Allocate(&m_ramBlock0MAIN, 0x1000);
		}

		m_memory.Clear();
	}

	void ComputerVIC20::InitTape()
	{
		m_tape.Init(1);
	}

	void ComputerVIC20::Reset()
	{
		ComputerBase::Reset();
		m_via1.Reset();
		m_via2.Reset();
		m_keyboard->Reset();
		m_tape.Reset();
	}

	bool ComputerVIC20::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		uint32_t cpuTicks = GetCPU().GetInstructionTicks();

		for (uint32_t i = 0; i < cpuTicks; ++i)
		{
			++g_ticks;

			if (!m_turbo)
			{
				SOUND().PlayMono(0/*m_via.GetSoundOut()*/);
			}

			// Tape update
			{
				TapeDeck& tape = m_tape.GetTape(0);

				//m_pia1.SetCassette1ReadLine(tape1.Read());
				//m_via.SetCassette2ReadLine(tape2.Read());
				//tape1.Write(m_via.GetCassetteDataOut());
				//tape2.Write(m_via.GetCassetteDataOut());
				//tape1.SetMotor(m_pia1.GetCassette1MotorOut());
				//tape2.SetMotor(m_via.GetCassette2MotorOut());
				//m_pia1.SetCassetteSense1In(tape1.GetSense());
				//m_pia1.SetCassetteSense2In(tape2.GetSense());

				m_tape.Tick();
			}

			GetInputs().Tick();
			if (GetInputs().IsQuit())
			{
				return false;
			}

			m_video->Tick();
			static bool oldBlank = false;
			bool blank = m_video->IsVSync();
			//if (blank != oldBlank)
			//{
			//	LogPrintf(LOG_DEBUG, "Blank: %d", blank);

			//	m_pia1.SetScreenRetrace(blank);
			//	m_via.SetRetraceIn(blank);
			//	oldBlank = blank;
			//}

			m_via1.Tick();
			m_via2.Tick();

			//GetCPU().SetIRQ(
			//	m_pia1.GetIRQA() ||
			//	m_pia1.GetIRQB() ||
			//	m_pia2.GetIRQA() ||
			//	m_pia2.GetIRQB() ||
			//	m_via.GetIRQ());
		}

		return true;
	}

	void ComputerVIC20::Serialize(json& to)
	{
		ComputerBase::Serialize(to);

		m_via1.Serialize(to["via1"]);
		m_via2.Serialize(to["via2"]);
	}

	void ComputerVIC20::Deserialize(const json& from)
	{
		ComputerBase::Deserialize(from);

		m_via1.Deserialize(from["via1"]);
		m_via2.Deserialize(from["via2"]);
	}
}
