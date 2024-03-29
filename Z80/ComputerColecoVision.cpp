#include "stdafx.h"

#include "ComputerColecoVision.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPUZ80.h"
#include "Video/VideoColecoVision.h"
#include <Sound/Sound.h>

using cfg::CONFIG;
using sound::SOUND;

namespace emul
{
	const size_t MAIN_CLK = 7159090;
	const size_t CPU_CLK = MAIN_CLK / 2;
	const size_t PIXEL_CLK = CPU_CLK * 3 / 2; // For reference only

	// Input scanning rate
	const size_t SCAN_CLK = 60; // TODO: Synchronize with vdp
	const size_t SCAN_RATE = CPU_CLK / SCAN_CLK;

	const ADDRESS RAM_SIZE = 1024;

	ComputerColecoVision::ComputerColecoVision() :
		Logger("ColecoVision"),
		ComputerBase(m_memory),
		m_ram("RAM", RAM_SIZE, emul::MemoryType::RAM),
		m_rom("ROM", 0x2000, emul::MemoryType::ROM),
		m_cart("CART", 0x8000, emul::MemoryType::ROM),
		m_sound(0xE0, CPU_CLK)
	{
	}

	void ComputerColecoVision::Reset()
	{
		ComputerBase::Reset();
		m_memory.Clear();
		m_joystick.Reset();
	}

	void ComputerColecoVision::Init(WORD baseRAM)
	{
		PortConnector::Init(PortConnectorMode::BYTE_LOW);
		ComputerBase::Init(CPUID_Z80, RAM_SIZE); // RAM is fixed

		InitJoystick();
		InitRAM();
		InitROM();
		InitSound();
		InitVideo();

		InitInputs(CPU_CLK, SCAN_RATE);

		GetInputs().InitKeyboard(&m_joystick);
		GetInputs().InitJoystick(&m_joystick);
	}

	void ComputerColecoVision::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_Z80) m_cpu = new CPUZ80(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void ComputerColecoVision::InitJoystick()
	{
		m_joystick.EnableLog(CONFIG().GetLogLevel("joystick"));
		m_joystick.Init();
	}

	void ComputerColecoVision::InitSound()
	{
		m_sound.EnableLog(CONFIG().GetLogLevel("sound"));
		m_sound.Init(32);
		SOUND().SetBaseClock(CPU_CLK);
	}

	void ComputerColecoVision::InitROM()
	{
		bool nodelay = CONFIG().GetValueBool("coleco", "nodelay");
		const char* bios = nodelay ? "data/z80/colecovision_nodelay.bin" : "data/z80/colecovision.bin";

		LogPrintf(LOG_INFO, "Loading BIOS ROM [%s]", bios);
		m_rom.LoadFromFile(bios);
		m_memory.Allocate(&m_rom, 0);

		m_cart.Clear(0xFF);
		m_memory.Allocate(&m_cart, 0x8000);

		std::string cart = CONFIG().GetValueStr("cartridge", "file");
		if (cart.size())
		{
			LoadCartridge(cart);
		}
	}

	void ComputerColecoVision::InitVideo()
	{
		video::VideoColecoVision* video = new video::VideoColecoVision();
		m_video = video;

		m_video->EnableLog(CONFIG().GetLogLevel("video"));
		m_video->Init(&m_memory, nullptr);
	}

	void ComputerColecoVision::InitRAM()
	{
		const ADDRESS ramBase = 0x6000;
		m_memory.Allocate(&m_ram, ramBase);
		for (ADDRESS offset = 1; offset < 8; ++offset)
		{
			m_memory.MapWindow(0x6000, ramBase + (RAM_SIZE * offset), RAM_SIZE);
		}
		m_memory.Clear();
	}

	void ComputerColecoVision::LoadCartridge(const std::filesystem::path& path)
	{
		UnloadCartridge();

		std::string cartFile = path.string();

		m_cartridgeInfo = path.stem().string();

		LogPrintf(LOG_INFO, "Loading cartridge image: %s", cartFile.c_str());
		m_cart.LoadFromFile(cartFile.c_str());
		Reset();
	}

	void ComputerColecoVision::UnloadCartridge()
	{
		m_cartridgeInfo.clear();
		m_cart.Clear(0xFF);
		Reset();
	}

	bool ComputerColecoVision::Step()
	{
		bool ready = m_sound.IsReady();
		uint32_t cpuTicks = 1;

		if (ready)
		{
			if (!ComputerBase::Step())
			{
				return false;
			}
			cpuTicks = GetCPU().GetInstructionTicks() + 1; // Add one wait state
		}

		for (uint32_t i = 0; i < cpuTicks; ++i)
		{
			++g_ticks;

			GetVideo().Tick();
			m_sound.Tick();

			if (!m_turbo)
			{
				SOUND().PlayMono(m_sound.GetOutput() * 10);
			}

			GetInputs().Tick();
			if (GetInputs().IsQuit())
			{
				return false;
			}

			GetCPU().SetNMI(GetVideo().IsInterrupt());
		}

		return true;
	}

	void ComputerColecoVision::Serialize(json& to)
	{
		ComputerBase::Serialize(to);
		m_sound.Serialize(to["sound"]);
		//m_joystick.Serialize(to["joystick"]);

		to["cartridgeInfo"] = m_cartridgeInfo;
	}
	void ComputerColecoVision::Deserialize(const json& from)
	{
		m_memory.Allocate(&m_cart, 0x8000);
		ComputerBase::Deserialize(from);
		m_sound.Deserialize(from["sound"]);
		//m_joystick.Deserialize(from["joystick"]);

		m_cartridgeInfo = from["cartridgeInfo"];
	}
}
