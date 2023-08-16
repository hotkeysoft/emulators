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
using namespace emul::Thomson;

using ScreenRAM = pia::thomson::ScreenRAM;

namespace emul
{
	ComputerThomson::ComputerThomson() :
		Logger("Thomson"),
		ComputerBase(m_memory, 64),
		IOConnector(63),
		m_pixelRAM("RAM_PIXEL", 0x2000, emul::MemoryType::RAM),
		m_attributeRAM("RAM_ATTR", 0x2000, emul::MemoryType::RAM),
		m_userRAM("RAM_USER", emul::MemoryType::RAM),
		m_osROM("ROM_OS", emul::MemoryType::ROM),
		m_basicROM("ROM_BASIC", 0x3000, emul::MemoryType::ROM),
		m_cartridgeROM("ROM_CART", emul::MemoryType::ROM),
		m_io("IO", 0x40)
	{
	}

	ComputerThomson::~ComputerThomson()
	{
		delete m_pia;
	}

	void ComputerThomson::Reset()
	{
		ComputerBase::Reset();
		GetVideo().Reset();
		m_pia->Reset();
		m_keyboard.Reset();
		m_lightpen.Reset();
		OnScreenMapChange(ScreenRAM::PIXEL);
	}

	void ComputerThomson::Init(WORD baseRAM)
	{
		ComputerBase::Init(emul::CPUID_6809, baseRAM);

		InitModel();
		InitROM();
		InitRAM();

		InitInputs(1000000, 20000);
		InitKeyboard();
		InitIO();

		InitVideo();
		InitLightpen();

		SOUND().SetBaseClock(1000000);
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

			m_osROM.Alloc(0x1000);
			m_osROM.LoadFromFile(osROM.c_str());
			m_memory.Allocate(&m_osROM, 0xF000);

			m_basicROM.LoadFromFile(basicROM.c_str());
			m_memory.Allocate(&m_basicROM, 0xC000);
			break;
		}
		case Model::TO7:
		{
			std::string osROM = m_basePathROM + "/TO7/to7.os.bin";

			m_osROM.Alloc(0x1800);
			m_osROM.LoadFromFile(osROM.c_str());
			m_memory.Allocate(&m_osROM, 0xE800);

			m_cartridgeROM.Alloc(0x4000);
			m_cartridgeROM.Clear(0xFF);
			m_memory.Allocate(&m_cartridgeROM, 0);
			break;
		}
		default:
			throw std::exception("not possible");
		}

		std::string cart = CONFIG().GetValueStr("cartridge", "file");
		if (cart.size())
		{
			LoadCartridge(cart);
		}
	}

	void ComputerThomson::InitRAM()
	{
		switch (m_model)
		{
		case Model::MO5:
			m_userRAM.Alloc(0x8000);
			m_memory.Allocate(&m_userRAM, 0x2000);
			m_screenRAMBase = 0;
			break;
		case Model::TO7:
			m_userRAM.Alloc(0x4000);
			m_memory.Allocate(&m_userRAM, 0x6000);
			m_screenRAMBase = 0x4000;
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
			m_memory.Allocate(&m_io, 0xA7C0);
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

		auto& video = GetVideo();
		for (uint32_t i = 0; i < cpuTicks; ++i)
		{
			++g_ticks;

			if (!m_turbo)
			{
				SOUND().PlayMono(m_pia->GetBuzzer() * 10000);
			}

			video.Tick();
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
		return {
			{"TO7 Cartridge (*.m7)", "*.m7"},
			{"Cartridge ROM (*.rom *.bin)", "*.rom;*.bin"}
		};
	}

	// TODO: Only TO7 for now
	void ComputerThomson::LoadCartridge(const std::filesystem::path& path)
	{
		UnloadCartridge();

		std::string cartFile = path.string();

		m_cartridgeInfo = path.stem().string();

		LogPrintf(LOG_INFO, "Loading cartridge image: %s", cartFile.c_str());
		m_cartridgeROM.LoadFromFile(cartFile.c_str());
		Reset();
	}
	void ComputerThomson::UnloadCartridge()
	{
		m_cartridgeInfo.clear();
		m_cartridgeROM.Clear(0xFF);
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
