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
	const std::map<std::string, ComputerCPC::Model> ComputerCPC::s_modelMap = {
		{"464", Model::CPC464},
		{"cpc464", Model::CPC464},
		{"664", Model::CPC664},
		{"cpc664", Model::CPC664},
		{"6128", Model::CPC6128},
		{"cpc6128", Model::CPC6128},
	};

	const size_t MAIN_CLK = 16000000; // 16 MHz Main crystal
	const size_t PIXEL_CLK = MAIN_CLK / 2;
	const size_t CPU_CLK = PIXEL_CLK / 2;

	// TODO: Sync with video
	const size_t RTC_CLK = 50; // 50Hz
	const size_t RTC_RATE = CPU_CLK / RTC_CLK;

	ComputerCPC::ComputerCPC() :
		Logger("cpc"),
		ComputerBase(m_memory),
		m_baseRAM("RAM", 0x10000, emul::MemoryType::RAM),
		m_romLow("ROM_L", ROM_SIZE, emul::MemoryType::ROM)
	{
	}

	ComputerCPC::~ComputerCPC()
	{
		delete m_tape;
		delete m_floppy;

		for (MemoryBlock* block : m_romBanks)
		{
			delete block;
		}
	}

	void ComputerCPC::Reset()
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

	bool ComputerCPC::LoadHighROM(BYTE bank, const char* romFile)
	{
		LogPrintf(LOG_DEBUG, "Load High ROM Bank [%d]: %s", bank, romFile);

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

	void ComputerCPC::Init(WORD baseRAM)
	{
		PortConnector::Init(PortConnectorMode::BYTE_HI);
		ComputerBase::Init(CPUID_Z80, baseRAM);

		InitModel();
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
			InitFloppy(new fdc::DeviceFloppyCPC(0x03F0, CPU_CLK));
		}
	}

	void ComputerCPC::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_Z80) m_cpu = new CPUZ80(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	ComputerCPC::Model ComputerCPC::StringToModel(const char* str)
	{
		auto m = s_modelMap.find(str);
		if (m != s_modelMap.end())
		{
			return m->second;
		}
		return Model::UNKNOWN;
	}
	std::string ComputerCPC::ModelToString(ComputerCPC::Model model)
	{
		for (auto curr : s_modelMap)
		{
			if (curr.second == model)
			{
				return curr.first.c_str();
			}
		}
		return "unknown";
	}

	void ComputerCPC::InitModel()
	{
		std::string model = CONFIG().GetValueStr("core", "model", "cpc464");

		m_model = StringToModel(model.c_str());

		if (m_model == Model::UNKNOWN)
		{
			m_model = Model::CPC464;
			LogPrintf(LOG_WARNING, "Unknown model [%s], using default", model.c_str());
		}

		LogPrintf(LOG_INFO, "InitModel: [%s]", ModelToString(m_model).c_str());
	}

	void ComputerCPC::InitKeyboard()
	{
		m_keyboard.EnableLog(CONFIG().GetLogLevel("keyboard"));
	}

	void ComputerCPC::InitJoystick()
	{
		if (CONFIG().GetValueBool("joystick", "enable"))
		{
			m_joystick.EnableLog(CONFIG().GetLogLevel("joystick"));
			m_joystick.Init();
		}
	}

	void ComputerCPC::InitRAM()
	{
		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_memory.Allocate(&m_baseRAM, 0);
	}

	void ComputerCPC::InitROM()
	{
		// TODO: Put in map/json file

		std::string osROM = m_basePathROM;
		std::string basicROM = m_basePathROM;
		std::string dosROM = m_basePathROM + "amstrad.amsdos.0.5.bin";

		switch (m_model)
		{
		case Model::CPC464:
			osROM.append("amstrad.cpc464.os.bin");
			basicROM.append("amstrad.cpc464.basic.1.0.bin");
			break;
		case Model::CPC664:
			osROM.append("amstrad.cpc664.os.bin");
			basicROM.append("amstrad.cpc664.basic.1.1.bin");
			break;
		case Model::CPC6128:
			osROM.append("amstrad.cpc6128.os.bin");
			basicROM.append("amstrad.cpc6128.basic.1.1.bin");
			break;
		default:
			throw std::exception("not possible");
		}

		if (!m_romLow.LoadFromFile(osROM.c_str()))
		{
			LogPrintf(LOG_ERROR, "Error loading OS ROM: %s", osROM.c_str());
			throw std::exception("Error loading OS ROM");
		}

		if (!LoadHighROM(0, basicROM.c_str()))
		{
			LogPrintf(LOG_ERROR, "Error loading BASIC ROM: %s", basicROM.c_str());
			throw std::exception("Error loading BASIC ROM");
		}

		if (CONFIG().GetValueBool("floppy", "enable"))
		{
			if (!LoadHighROM(7, dosROM.c_str()))
			{
				LogPrintf(LOG_ERROR, "Error loading DOS ROM: %s", dosROM.c_str());
			}
		}

		// Only enable low rom (os) at boot
		OnLowROMChange(true); // Load low ROM on top of RAM
		OnHighROMChange(false);

		Connect("xx0xxxxx", static_cast<PortConnector::OUTFunction>(&ComputerCPC::SelectROMBank));
	}

	void ComputerCPC::InitIO()
	{
		m_pio.EnableLog(CONFIG().GetLogLevel("pio"));
		m_pio.SetKeyboard(&m_keyboard);
		m_pio.SetJoystick(&m_joystick);
		m_pio.SetSound(&m_sound);
		m_pio.Init("xxxx0xxx", true);
	}

	void ComputerCPC::InitSound()
	{
		SOUND().SetBaseClock(CPU_CLK);
		m_sound.EnableLog(CONFIG().GetLogLevel("sound"));
		m_sound.Init();
	}

	void ComputerCPC::InitVideo()
	{
		// Gate array always read directly from base ram block
		m_video = new video::cpc::VideoCPC(&m_baseRAM);

		GetVideo().SetEventHandler(this);

		m_video->EnableLog(CONFIG().GetLogLevel("video"));
		m_video->Init(&m_memory, nullptr);
	}

	void ComputerCPC::InitTape()
	{
		m_tape = new tape::DeviceTape(CPU_CLK);
		m_tape->Init(1);
	}

	void ComputerCPC::InitFloppy(fdc::DeviceFloppy* fdd)
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

	void ComputerCPC::OnLowROMChange(bool load)
	{
		LoadROM(load, &m_romLow, ROM_LOW);
	}

	void ComputerCPC::OnHighROMChange(bool load)
	{
		m_highROMLoaded = load;
		LoadROM(load, GetCurrHighROM(), ROM_HIGH);
	}

	void ComputerCPC::LoadROM(bool load, MemoryBlock* rom, ADDRESS base)
	{
		LogPrintf(LOG_DEBUG, "%sLOAD ROM [%s]", (load ? "" : "UN"), rom->GetId().c_str());

		if (load)
		{
			m_memory.Allocate(rom, base, -1, emul::AllocateMode::READ);
		}
		else
		{
			m_memory.Restore(base, ROM_SIZE, emul::AllocateMode::READ);
		}
	}

	void ComputerCPC::SelectROMBank(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Select ROM Bank: [%d]", value);
		m_currHighROM = value;

		if (m_highROMLoaded)
		{
			LoadROM(true, GetCurrHighROM(), ROM_HIGH);
		}
	}

	void ComputerCPC::TickFloppy()
	{
		if (!m_floppy)
		{
			return;
		}

		m_floppy->Tick();
	}

	bool ComputerCPC::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		uint32_t cpuTicks = GetCPU().GetInstructionTicks();

		// Stretch ticks so they are a multiple of 1us (4 ticks)
		cpuTicks = (cpuTicks + 3) & ~0x3;

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

			m_pio.SetVSync(GetVideo().IsVSync());
		}
		GetCPU().SetINT(GetVideo().IsInterrupt());

		return true;
	}

	void ComputerCPC::Serialize(json& to)
	{
		ComputerBase::Serialize(to);
		to["model"] = ModelToString(m_model);


		if (m_floppy)
		{
			m_floppy->Serialize(to["floppy"]);
		}
	}

	void ComputerCPC::Deserialize(const json& from)
	{
		ComputerBase::Deserialize(from);
		std::string modelStr = from["model"];
		Model model = StringToModel(modelStr.c_str());
		if ((m_model == Model::UNKNOWN) || (model != m_model))
		{
			throw SerializableException("Computer: Model is not compatible", SerializationError::COMPAT);
		}

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
