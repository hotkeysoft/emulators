#include "stdafx.h"

#include "ComputerCPC464.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPUZ80.h"
#include "Video/VideoCPC464.h"
#include "Storage/DeviceFloppyCPC464.h"
#include <Sound/Sound.h>

using cfg::CONFIG;
using sound::SOUND;
using tape::TapeDeck;

namespace emul
{
	// TODO
	const size_t MAIN_CLK = 16000000; // 16 MHz Main crystal
	const size_t PIXEL_CLK = MAIN_CLK / 2;
	const size_t CPU_CLK = PIXEL_CLK / 2;

	// TODO: Sync with video
	const size_t RTC_CLK = 50; // 50Hz
	const size_t RTC_RATE = CPU_CLK / RTC_CLK;

	ComputerCPC464::ComputerCPC464() :
		Logger("ZXSpectrum"),
		ComputerBase(m_memory),
		m_baseRAM("RAM", 0x10000, emul::MemoryType::RAM),
		m_romLow("ROM_L", ROM_SIZE, emul::MemoryType::ROM)
	{
	}

	ComputerCPC464::~ComputerCPC464()
	{
		delete m_tape;
		delete m_floppy;

		for (MemoryBlock* block : m_romBanks)
		{
			delete block;
		}
	}

	void ComputerCPC464::Reset()
	{
		ComputerBase::Reset();
		GetVideo().Reset();
		m_pio.Reset();
		m_keyboard.Reset();
		m_tape->Reset();
		m_sound.Reset();

		OnLowROMChange(true); // Load low ROM on top of RAM
		m_currHighROM = 0;
		OnHighROMChange(false);
	}

	bool ComputerCPC464::LoadHighROM(BYTE bank, const char* romFile)
	{
		LogPrintf(LOG_INFO, "Load High ROM Bank [%d]: %s", bank, romFile);

		char id[16];
		sprintf(id, "ROM_H%02X", bank);

		delete(m_romBanks[bank]);
		m_romBanks[bank] = nullptr;

		MemoryBlock* block = new MemoryBlock(id, ROM_SIZE, emul::MemoryType::ROM);

		bool loaded = block->LoadFromFile(romFile);
		if (loaded)
		{
			m_romBanks[bank] = block;
		}
		else
		{
			delete block;
		}

		return loaded;
	}

	void ComputerCPC464::Init(WORD baseRAM)
	{
		PortConnector::Init(PortConnectorMode::BYTE_HI);
		ComputerBase::Init(CPUID_Z80, baseRAM);

		InitRAM();
		InitROM();
		InitKeyboard();
		InitJoystick();
		InitSound();
		InitIO();
		InitVideo();
		InitTape();

		InitInputs(CPU_CLK, RTC_RATE);

		GetInputs().InitKeyboard(&m_keyboard);
		GetInputs().InitJoystick(&m_joystick);

		if (CONFIG().GetValueBool("floppy", "enable"))
		{
			InitFloppy(new fdc::DeviceFloppyCPC464(0x03F0, CPU_CLK));
		}
	}

	void ComputerCPC464::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_Z80) m_cpu = new CPUZ80(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void ComputerCPC464::InitKeyboard()
	{
		m_keyboard.EnableLog(CONFIG().GetLogLevel("keyboard"));
	}

	void ComputerCPC464::InitJoystick()
	{
		if (CONFIG().GetValueBool("joystick", "enable"))
		{
			m_joystick.EnableLog(CONFIG().GetLogLevel("joystick"));
			m_joystick.Init();
		}
	}

	void ComputerCPC464::InitRAM()
	{
		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_memory.Allocate(&m_baseRAM, 0);
	}

	void ComputerCPC464::InitROM()
	{
		m_romLow.LoadFromFile("data/z80/amstrad.cpc464.os.bin");
		LoadHighROM(0, "data/z80/amstrad.cpc464.basic.bin");

		// TODO: Dynamic
		LoadHighROM(7, "data/z80/amstrad.amsdos.0.5.bin");

		// Only enable low rom (os) at boot
		OnLowROMChange(true); // Load low ROM on top of RAM
		OnHighROMChange(false);

		Connect("xx0xxxxx", static_cast<PortConnector::OUTFunction>(&ComputerCPC464::SelectROMBank));
	}

	void ComputerCPC464::InitIO()
	{
		m_pio.EnableLog(CONFIG().GetLogLevel("pio"));
		m_pio.SetKeyboard(&m_keyboard);
		m_pio.SetJoystick(&m_joystick);
		m_pio.SetSound(&m_sound);
		m_pio.Init("xxxx0xxx");
	}

	void ComputerCPC464::InitSound()
	{
		SOUND().SetBaseClock(CPU_CLK);
		m_sound.EnableLog(CONFIG().GetLogLevel("sound"));
		m_sound.Init();
	}

	void ComputerCPC464::InitVideo()
	{
		// Gate array always read directly from base ram block
		vid464::VideoCPC464* video = new vid464::VideoCPC464(&m_baseRAM);
		m_video = video;

		GetVideo().SetEventHandler(this);

		m_video->EnableLog(CONFIG().GetLogLevel("video"));
		m_video->Init(&m_memory, nullptr);
	}

	void ComputerCPC464::InitTape()
	{
		m_tape = new tape::DeviceTape(CPU_CLK);
		m_tape->Init(1);
	}

	void ComputerCPC464::InitFloppy(fdc::DeviceFloppy* fdd)
	{
		assert(fdd);
		m_floppy = fdd;

		m_floppy->EnableLog(CONFIG().GetLogLevel("floppy"));
		m_floppy->Init();

		std::string floppy = CONFIG().GetValueStr("floppy", "floppy.1");
		if (floppy.size())
		{
			m_floppy->LoadDiskImage(0, floppy.c_str());
		}

		floppy = CONFIG().GetValueStr("floppy", "floppy.2");
		if (floppy.size())
		{
			m_floppy->LoadDiskImage(1, floppy.c_str());
		}
	}

	void ComputerCPC464::OnLowROMChange(bool load)
	{
		LoadROM(load, &m_romLow, ROM_LOW);
	}

	void ComputerCPC464::OnHighROMChange(bool load)
	{
		m_highROMLoaded = load;
		LoadROM(load, GetCurrHighROM(), ROM_HIGH);
	}

	void ComputerCPC464::LoadROM(bool load, MemoryBlock* rom, ADDRESS base)
	{
		LogPrintf(LOG_INFO, "%sLOAD ROM [%s]", (load ? "" : "UN"), rom->GetId().c_str());

		if (load)
		{
			m_memory.Allocate(rom, base, -1, emul::AllocateMode::READ);
		}
		else
		{
			m_memory.Restore(base, ROM_SIZE, emul::AllocateMode::READ);
		}
	}

	void ComputerCPC464::SelectROMBank(BYTE value)
	{
		LogPrintf(LOG_INFO, "Select ROM Bank: [%d]", value);
		m_currHighROM = value;

		if (m_highROMLoaded)
		{
			LoadROM(true, GetCurrHighROM(), ROM_HIGH);
		}
	}

	void ComputerCPC464::TickFloppy()
	{
		if (!m_floppy)
		{
			return;
		}

		m_floppy->Tick();
	}

	bool ComputerCPC464::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		uint32_t cpuTicks = GetCPU().GetInstructionTicks();

		if (GetCPU().GetState() != CPUState::HALT)
		{
			// Stretch ticks so they are a multiple of 1us (4 ticks)
			cpuTicks = (cpuTicks + 3) & ~0x3;
		}

		if (GetCPU().IsInterruptAcknowledge())
		{
			GetVideo().InterruptAcknowledge();
		}

		TapeDeck& tape = m_tape->GetTape(0);

		for (uint32_t i = 0; i < cpuTicks; ++i)
		{
			++g_ticks;

			GetVideo().Tick();

			if (!m_turbo)
			{
				//SOUND().PlayMono(tape.Read() * 8000);
				const auto& out = m_sound.GetOutput();
				SOUND().PlayStereo(out.A + out.B, out.A + out.C);
			}

			{
				m_pio.SetCassetteInput(tape.Read());
				tape.Write(m_pio.GetCassetteOutput());
				tape.SetMotor(m_pio.GetCassetteMotorOut());

				m_tape->Tick();
			}

			GetInputs().Tick();
			if (GetInputs().IsQuit())
			{
				return false;
			}

			TickFloppy();

			m_sound.Tick();

			GetCPU().SetINT(GetVideo().IsInterrupt());
			m_pio.SetVSync(GetVideo().IsVSync());
		}

		return true;
	}

	void ComputerCPC464::Serialize(json& to)
	{
		ComputerBase::Serialize(to);

		if (m_floppy)
		{
			m_floppy->Serialize(to["floppy"]);
		}
	}

	void ComputerCPC464::Deserialize(const json& from)
	{
		ComputerBase::Deserialize(from);

		if ((from.contains("floppy") && !m_floppy) ||
			(!from.contains("floppy") && m_floppy))
		{
			throw SerializableException("Computer: Floppy configuration is not compatible", SerializationError::COMPAT);
		}

		if (m_floppy)
		{
			m_floppy->Deserialize(from["floppy"]);
		}
	}
}
