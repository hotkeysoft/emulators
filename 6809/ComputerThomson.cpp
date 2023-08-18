#include "stdafx.h"

#include "ComputerThomson.h"
#include <Config.h>
#include <Sound/Sound.h>
#include "IO/Console.h"
#include "CPU/CPU6809.h"
#include "Hardware/Device6520MO5_PIA.h"
#include "Hardware/DevicePIAThomsonTO7.h"

using cfg::CONFIG;
using sound::SOUND;
using pia::thomson::Device6520MO5_PIA;
using pia::thomson::DevicePIAThomsonTO7;
using tape::TapeDeck;
using namespace emul::Thomson;

using ScreenRAM = pia::thomson::ScreenRAM;

namespace emul
{
	ComputerThomson::ComputerThomson() :
		Logger("Thomson"),
		ComputerBase(m_memory, IO_BLOCK_SIZE),
		IOConnector(IO_BLOCK_SIZE - 1),
		m_pixelRAM("RAM_PIXEL", RAM_SCREEN_SIZE, emul::MemoryType::RAM),
		m_attributeRAM("RAM_ATTR", RAM_SCREEN_SIZE, emul::MemoryType::RAM),
		m_userRAM("RAM_USER", emul::MemoryType::RAM),
		m_extRAM("RAM_EXT", emul::MemoryType::RAM),
		m_osROM("ROM_OS", emul::MemoryType::ROM),
		m_basicROM("ROM_BASIC", MO5_ROM_BASIC_SIZE, emul::MemoryType::ROM), // MO5
		m_cartridgeROM("ROM_CART", ROM_CARTRIDGE_SIZE, emul::MemoryType::ROM),
		m_io("IO", IO_BLOCK_SIZE)
	{
	}

	ComputerThomson::~ComputerThomson()
	{
		delete m_pia;
		delete m_tape;
	}

	void ComputerThomson::Reset()
	{
		ComputerBase::Reset();
		if (m_video)
		{
			m_video->Reset();
		}
		if (m_pia)
		{
			m_pia->Reset();
		}
		if (m_tape)
		{
			m_tape->Reset();
		}

		m_keyboard.Reset();
		m_lightpen.Reset();
		OnScreenMapChange(ScreenRAM::PIXEL);
	}

	void ComputerThomson::Init(WORD baseRAM)
	{
		ComputerBase::Init(emul::CPUID_6809, baseRAM);

		InitModel();
		InitROM();
		InitRAM(baseRAM);

		InitInputs(CPU_CLOCK, INPUT_REFRESH_PERIOD);
		InitKeyboard();
		InitIO();

		InitVideo();
		InitLightpen();
		InitTape();

		SOUND().SetBaseClock(CPU_CLOCK);

		InitCartridge();
	}

	void ComputerThomson::InitCartridge()
	{
		std::string cart = CONFIG().GetValueStr("cartridge", "file");
		if (cart.empty())
		{
			switch (m_model)
			{
			case Model::MO5: cart = CONFIG().GetValueStr("cartridge.mo5", "file"); break;
			case Model::TO7: cart = CONFIG().GetValueStr("cartridge.to7", "file"); break;
			}
		}

		if (cart.size())
		{
			LoadCartridge(cart);
		}
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

	void ComputerThomson::InitModel()
	{
		std::string model = CONFIG().GetValueStr("core", "model", "mo5");

		m_model = StringToModel(model.c_str());

		if (m_model == Model::UNKNOWN)
		{
			m_model = Model::MO5;
			LogPrintf(LOG_WARNING, "Unknown model [%s], using default", model.c_str());
		}

		LogPrintf(LOG_INFO, "InitModel: [%s]", ModelToString(m_model).c_str());
	}

	void ComputerThomson::InitROM()
	{
		switch (m_model)
		{
		case Model::MO5:
		{
			std::string osROM = m_basePathROM + "/MO5/mo5.os.bin";
			std::string basicROM = m_basePathROM + "/MO5/mo5.basic.bin";

			m_osROM.Alloc(MO5_ROM_MONITOR_SIZE);
			m_osROM.LoadFromFile(osROM.c_str());
			m_memory.Allocate(&m_osROM, MO5_ROM_MONITOR_BASE);

			m_basicROM.LoadFromFile(basicROM.c_str());
			m_memory.Allocate(&m_basicROM, MO5_ROM_BASIC_BASE);

			break;
		}
		case Model::TO7:
		{
			std::string osROM = m_basePathROM + "/TO7/to7.os.bin";

			m_osROM.Alloc(TO7_ROM_MONITOR_SIZE);
			m_osROM.LoadFromFile(osROM.c_str());
			m_memory.Allocate(&m_osROM, TO7_ROM_MONITOR_BASE);

			m_cartridgeROM.Clear(0xFF);
			m_memory.Allocate(&m_cartridgeROM, TO7_ROM_CARTRIDGE_BASE);
			break;
		}
		default:
			throw std::exception("not possible");
		}
	}

	void ComputerThomson::InitRAM(WORD baseRAM)
	{
		LogPrintf(LOG_INFO, "Requested RAM: %dKB", baseRAM);

		switch (m_model)
		{
		case Model::MO5:
			m_userRAM.Alloc(MO5_RAM_USER_SIZE);
			m_memory.Allocate(&m_userRAM, MO5_RAM_USER_BASE);
			m_screenRAMBase = MO5_RAM_SCREEN_BASE;
			m_baseRAMSize = 32;
			break;
		case Model::TO7:
			if (!baseRAM)
			{
				baseRAM = 24;
			}

			LogPrintf(LOG_INFO, "Allocating base RAM (8KB)");
			m_userRAM.Alloc(TO7_RAM_USER_SIZE);
			m_memory.Allocate(&m_userRAM, TO7_RAM_USER_BASE);
			m_screenRAMBase = TO7_RAM_SCREEN_BASE;
			m_baseRAMSize = 8;

			// Check for 16 K extension
			if (baseRAM == 8)
			{
				break; // nothing to do, already allocated
			}
			else if (baseRAM < 8)
			{
				LogPrintf(LOG_WARNING, "Requested RAM < 8KB, using 8KB");
				break; // nothing to do, already allocated
			}
			else if (baseRAM > 24)
			{
				LogPrintf(LOG_WARNING, "Requested RAM > 24KB, using 24KB");
				baseRAM = 24;
			}

			// Nothing to do for 8k, already allocated
			if (baseRAM != 24)
			{
				LogPrintf(LOG_WARNING, "Requested RAM should be 8 or 24, using 24KB");
			}

			m_baseRAMSize = 24;
			LogPrintf(LOG_INFO, "Allocating Extension RAM (16KB)");
			m_extRAM.Alloc(TO7_RAM_EXT_SIZE);
			m_memory.Allocate(&m_extRAM, TO7_RAM_EXT_BASE);
			break;
		default:
			throw std::exception("not possible");
		}

		m_userRAM.Clear(0xF0);
		m_pixelRAM.Clear(0x0F);
		m_attributeRAM.Clear(0xF0);
	}

	void ComputerThomson::InitKeyboard()
	{
		m_keyboard.SetModel(m_model);
		GetInputs().InitKeyboard(&m_keyboard);
	}

	void ComputerThomson::InitIO()
	{
		switch (m_model)
		{
		case Model::MO5:
		{
			Device6520MO5_PIA* pia = new Device6520MO5_PIA();
			pia->EnableLog(CONFIG().GetLogLevel("pia"));
			pia->Init(&m_keyboard);
			m_io.AddDevice(*pia, 0, 0b111100); // A7C0-A7C3
			m_memory.Allocate(&m_io, MO5_IO_BASE);
			m_pia = pia;
			break;
		}
		case Model::TO7:
		{
			DevicePIAThomsonTO7* pia = new DevicePIAThomsonTO7();
			pia->GetPIA1().EnableLog(CONFIG().GetLogLevel("pia"));
			pia->GetPIA2().EnableLog(CONFIG().GetLogLevel("pia.2"));
			pia->Init(&m_keyboard);

			m_memory.Allocate(&m_io, 0xE7C0); // E7C0-E7FF

			m_io.AddDevice(pia->GetPIA1(), "000xxx"); // 0xE7C0-E7C7 (base=E7C0)
			m_io.AddDevice(pia->GetPIA2(), "0010xx"); // 0xE7C8-E7CB (base=E7C0)

			m_pia = pia;
			break;
		}
		default:
			throw std::exception("not possible");
		}
		m_pia->SetPIAEventHandler(this);
	}

	void ComputerThomson::InitLightpen()
	{
		m_lightpen.EnableLog(CONFIG().GetLogLevel("mouse"));
		m_lightpen.SetPIA(m_pia);
		m_lightpen.SetVideo(&GetVideo());
		GetInputs().InitMouse(&m_lightpen);
	}

	void ComputerThomson::InitTape()
	{
		m_tape = new tape::DeviceTape(CPU_CLOCK);
		m_tape->Init(1);
	}

	void ComputerThomson::OnScreenMapChange(ScreenRAM map)
	{
		LogPrintf(LOG_DEBUG, "OnScreenMapChange PIXEL=%d", map);
		m_memory.Allocate((map == ScreenRAM::PIXEL) ? &m_pixelRAM : &m_attributeRAM, m_screenRAMBase);
	}

	void ComputerThomson::OnBorderChange(BYTE borderRGBP)
	{
		LogPrintf(LOG_DEBUG, "OnBorderChange: %X", borderRGBP);

		GetVideo().SetBorderColor(borderRGBP);
	}

	void ComputerThomson::InitVideo()
	{
		video::VideoThomson* video = new video::VideoThomson();
		video->EnableLog(CONFIG().GetLogLevel("video"));

		video->Init(m_model, &m_pixelRAM, &m_attributeRAM);
		m_video = video;

		constexpr uint32_t DEFAULT_BORDER = 16;
		uint32_t border = CONFIG().GetValueInt32("video", "border", DEFAULT_BORDER);
		if (border < 0 && border > 64)
		{
			border = DEFAULT_BORDER;
		}
		m_video->SetBorder(border);

		switch (m_model)
		{
		case Model::MO5:
			// 0xA7E4..0xA7E7 (base = 0xA7C0)
			m_io.AddDevice(*video, 0b100100, 0b111100);
			break;
		case Model::TO7: // No gate array on TO7
			break;
		default:
			throw std::exception("not possible");
		}
	}

	bool ComputerThomson::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		uint32_t cpuTicks = GetCPU().GetInstructionTicks();

		TapeDeck& tape = m_tape->GetTape(0);
		auto& video = GetVideo();

		for (uint32_t i = 0; i < cpuTicks; ++i)
		{
			++g_ticks;

			if (!m_turbo)
			{
				SOUND().PlayMono(tape.GetSound() + (m_pia->GetBuzzer() * 4000));
			}

			{
				tape.SetMotor(m_pia->GetTapeMotorState());
				m_pia->SetCassetteInput(tape.Read() ^ tape.GetSense());
				tape.Write(m_pia->GetCassetteOut());

				m_tape->Tick();
			}

			video.Tick();
			m_pia->Tick();
			m_pia->SetVSync(video.IsVSync());

			if (video.IsLightpen())
			{
				m_pia->TriggerLightPenInterrupt();
			}

			GetCPU().SetIRQ(m_pia->GetIRQ());
			GetCPU().SetFIRQ(m_pia->GetFIRQ());

			GetInputs().Tick();
			if (GetInputs().IsQuit())
			{
				return false;
			}

		}

		return true;
	}

	hscommon::fileUtil::SelectFileFilters ComputerThomson::GetLoadFilter()
	{
		switch (m_model)
		{
		case Model::MO5:
			return {
				{"MO5 Cartridge (*.m5)", "*.m5"},
				{"Cartridge ROM (*.rom *.bin)", "*.rom;*.bin"}
			};
		case Model::TO7:
			return {
				{"TO7 Cartridge (*.m7)", "*.m7"},
				{"Cartridge ROM (*.rom *.bin)", "*.rom;*.bin"}
			};
		default:
			LogPrintf(LOG_ERROR, "Unknown model");
			return CartridgeLoader::GetLoadFilter();
		}
	}

	// TODO: Only TO7 for now
	void ComputerThomson::LoadCartridge(const std::filesystem::path& path)
	{
		UnloadCartridge();

		std::string cartFile = path.string();

		m_cartridgeInfo = path.stem().string();

		LogPrintf(LOG_INFO, "Loading cartridge image: %s", cartFile.c_str());
		m_cartridgeROM.LoadFromFile(cartFile.c_str());

		// On the MO5, need to swap out BASIC
		if (m_model == Model::MO5)
		{
			m_memory.Free(&m_basicROM);
			m_memory.Allocate(&m_cartridgeROM, MO5_ROM_CARTRIDGE_BASE);
		}

		Reset();
	}
	void ComputerThomson::UnloadCartridge()
	{
		m_cartridgeInfo.clear();
		m_cartridgeROM.Clear(0xFF);

		// On the MO5, need to put BASIC back
		if (m_model == Model::MO5)
		{
			m_memory.Free(&m_cartridgeROM);
			m_memory.Allocate(&m_basicROM, MO5_ROM_BASIC_BASE);
		}

		Reset();
	}

	void ComputerThomson::Serialize(json& to)
	{
		ComputerBase::Serialize(to);
		to["model"] = ModelToString(m_model);

		m_pia->Serialize(to["pia"]);
	}

	void ComputerThomson::Deserialize(const json& from)
	{
		ComputerBase::Deserialize(from);
		std::string modelStr = from["model"];
		Model model = StringToModel(modelStr.c_str());
		if ((m_model == Model::UNKNOWN) || (model != m_model))
		{
			throw SerializableException("Computer: Model is not compatible", SerializationError::COMPAT);
		}

		m_pia->Deserialize(from["pia"]);
	}
}
