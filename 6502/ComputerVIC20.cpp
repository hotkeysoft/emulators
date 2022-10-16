#include "stdafx.h"

#include "ComputerVIC20.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU6502.h"
#include <Sound/Sound.h>
#include "IO/DeviceKeyboardVIC20.h"
#include "Video/VideoVIC.h"

using cfg::CONFIG;
using sound::SOUND;
using tape::TapeDeck;
using video::vic::VideoVIC;
using sound::vic::SoundVIC;

namespace emul
{
	const size_t MAIN_CLK = 14318180; // Main crystal
	const size_t PIXEL_CLK = (MAIN_CLK / 7) * 2;
	const size_t CPU_CLK = PIXEL_CLK / 4;

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
		InitJoystick();
		InitRAM(baseRAM);
		InitROM();
		InitIO();
		InitSound();
		InitVideo();
		InitTape();

		InitInputs(CPU_CLK, SCAN_RATE);
		GetInputs().InitKeyboard(m_keyboard);
		GetInputs().InitJoystick(&m_joystick);

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
		m_keyboard = new kbd::DeviceKeyboardVIC20();
		m_keyboard->EnableLog(CONFIG().GetLogLevel("keyboard"));
	}

	void ComputerVIC20::InitJoystick()
	{
		if (CONFIG().GetValueBool("joystick", "enable"))
		{
			m_joystick.EnableLog(CONFIG().GetLogLevel("joystick"));
			m_joystick.Init();
		}
	}

	void ComputerVIC20::InitSound()
	{
		m_sound.EnableLog(CONFIG().GetLogLevel("sound"));
		m_sound.Init();
	}

	void ComputerVIC20::InitROM()
	{
		m_romCHAR.LoadFromFile("data/VIC20/CHAR-8000.bin");
		m_memory.Allocate(&m_romCHAR, 0x8000);

		m_romBASIC.LoadFromFile("data/VIC20/BASIC-C000.bin");
		m_memory.Allocate(&m_romBASIC, 0xC000);

		m_romKERNAL.LoadFromFile("data/VIC20/KERNAL-E000-ntsc.bin");
		m_memory.Allocate(&m_romKERNAL, 0xE000);

		// 16K cartridge
		//MemoryBlock* rom6000 = new MemoryBlock("ROM6000", 8192, MemoryType::ROM);
		//rom6000->LoadFromFile("D:/Dloads/Emulation/VIC20/Games/16k/6000+A000/cart/Pole Position-6000.prg");
		//m_memory.Allocate(rom6000, 0x6000);

		//MemoryBlock* romA000 = new MemoryBlock("ROMA000", 8192, MemoryType::ROM);
		//romA000->LoadFromFile("D:/Dloads/Emulation/VIC20/Games/16k/6000+A000/cart/Pole Position-a000.prg");
		//m_memory.Allocate(romA000, 0xA000);

		// 8K cartridge
		//MemoryBlock* romA000 = new MemoryBlock("ROMA000", 8192, MemoryType::ROM);
		//romA000->LoadFromFile("D:/Dloads/Emulation/VIC20/Games/8k/cart/OmegaRaceOrig.prg");
		//m_memory.Allocate(romA000, 0xA000);
	}

	void ComputerVIC20::InitVideo()
	{
		VideoVIC* video = new VideoVIC(m_sound);
		video->EnableLog(CONFIG().GetLogLevel("video"));
		video->Init(&m_memory, nullptr);
		m_video = video;

		// Map it to memory
		m_memory.Allocate(&m_ioVIC, 0x9000);
		m_ioVIC.AddDevice(*video, 0);
	}

	void ComputerVIC20::InitIO()
	{
		m_ioVIA.EnableLog(LOG_ERROR);

		m_memory.Allocate(&m_ioVIA, 0x9100);

		// VIA1 @ 0x91[10]
		m_via1.EnableLog(CONFIG().GetLogLevel("vic20.via1"));
		m_via1.Init(&m_joystick);
		// Incomplete decoding, will also select at 3x, 5x, 7x etc
		m_ioVIA.AddDevice(m_via1, 0x10);

		// VIA2 @ 0x91[20]
		m_via2.EnableLog(CONFIG().GetLogLevel("vic20.via2"));
		m_via2.Init(m_keyboard, &m_joystick);
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

		// Above 8K, Round to 8k block
		if (baseRAM > 5)
		{
			WORD rounded = ((baseRAM + 7) & 0xF8);
			if (rounded != baseRAM)
			{
				baseRAM = rounded;
				LogPrintf(LOG_WARNING, "Requested base RAM rounded to (%dKB)", baseRAM);
			}
		}

		if (baseRAM > 32)
		{
			baseRAM = 32;
			LogPrintf(LOG_WARNING, "Requested base RAM too high (%dKB), using 32KB", baseRAM);
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
			m_memory.Allocate(&m_ramBlock0RAM2, 0x0800);
			m_memory.Allocate(&m_ramBlock0RAM1, 0x0400);
			[[fallthrough]];
		default:
			// Base 5K: Always allocated
			m_memory.Allocate(&m_ramBlock0LOW, 0x0000);
			m_memory.Allocate(&m_ramBlock0MAIN, 0x1000);
		}

		m_memory.Allocate(&m_ramCOLOR, 0x9400);

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
				SOUND().PlayMono(m_sound.GetOutput());
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
			m_sound.Tick();

			m_via1.Tick();
			m_via2.Tick();

			GetCPU().SetNMI(m_via1.GetIRQ());
			GetCPU().SetIRQ(m_via2.GetIRQ());
		}

		return true;
	}

	void ComputerVIC20::LoadPRG(const char* file)
	{
		LogPrintf(LOG_INFO, "LoadPRG: loading %s", file);

		FILE* f = fopen(file, "rb");
		if (!f)
		{
			LogPrintf(LOG_ERROR, "LoadPRG: error opening binary file");
			return;
		}

		// Load "header" (2 bytes, load address)
		WORD loadAddress = 0;
		size_t bytesRead = fread(&loadAddress, 2, 1, f);
		if (bytesRead != 1)
		{
			LogPrintf(LOG_ERROR, "LoadPRG: error reading header");
			return;
		}
		else
		{
			LogPrintf(LOG_INFO, "Load Address: %04X", loadAddress);
		}

		// TODO: Cartridges: allocate ROM block(s)

		// Load data
		MemoryBlock::RawBlock buf;
		buf.resize(32768);
		bytesRead = fread(&buf[0], sizeof(BYTE), buf.size(), f);
		if (bytesRead < 1)
		{
			LogPrintf(LOG_ERROR, "LoadPRG: error reading binary file");
			return;
		}

		LogPrintf(LOG_INFO, "LoadPRG: read %d bytes", bytesRead);
		buf.resize(bytesRead);

		// Find memory block where we want to insert the code
		const MemorySlot& slot = m_memory.FindBlock(loadAddress);
		MemoryBlock* block = dynamic_cast<MemoryBlock*>(slot.block);
		if (!block)
		{
			LogPrintf(LOG_ERROR, "LoadPRG: No memory allocated at load address: %04X");
			return;
		}

		block->Fill(loadAddress - slot.base, buf);

		WORD end = loadAddress + (WORD)bytesRead;

		WORD basicStart = m_memory.Read16(0x2B);

		if (loadAddress == basicStart)
		{
			LogPrintf(LOG_INFO, "LoadPRG: Adjusting BASIC pointers");

			m_memory.Write16(0x2D, end);
			m_memory.Write16(0x2F, end);
			m_memory.Write16(0x31, end);
		}

		fclose(f);
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
