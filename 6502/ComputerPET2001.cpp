#include "stdafx.h"

#include "ComputerPET2001.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU6502.h"
#include <Sound/Sound.h>

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

	ComputerPET2001::ComputerPET2001() :
		Logger("PET2001"),
		ComputerBase(m_memory, 256),
		m_baseRAM("RAM"),
		m_romC000("ROMC0000", 0x1000, emul::MemoryType::ROM),
		m_romD000("ROMD0000", 0x1000, emul::MemoryType::ROM),
		m_romE000("ROME0000", 0x0800, emul::MemoryType::ROM),
		m_romF000("ROMF0000", 0x1000, emul::MemoryType::ROM),
		m_videoRAM("VID", 0x0400),
		m_ioE800("IO", 0x0100),
		m_pia1("PIA1"),
		m_pia2("PIA2"),
		m_tape(CPU_CLK)
	{
	}

	void ComputerPET2001::Init(WORD baseRAM)
	{
		ComputerBase::Init(CPUID_6502, baseRAM);

		InitModel();
		InitRAM(baseRAM);
		InitROM();
		InitIO();
		InitVideo();
		InitTape();

		InitInputs(CPU_CLK, SCAN_RATE);
		GetInputs().InitKeyboard(&m_keyboard);

		SOUND().SetBaseClock(CPU_CLK);
	}

	void ComputerPET2001::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_6502) m_cpu = new CPU6502(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void ComputerPET2001::InitModel()
	{
		std::string model = CONFIG().GetValueStr("core", "model", "basic1p");

		m_model = Model::BASIC1p;

		const std::map<std::string, Model> models = {
			{"basic1", Model::BASIC1},
			{"basic1p", Model::BASIC1p},
			{"basic2n", Model::BASIC2n},
			{"basic2p", Model::BASIC2p}
		};

		auto m = models.find(model);
		if (m != models.end())
		{
			m_model = m->second;
		}
		else
		{
			LogPrintf(LOG_WARNING, "Unknown model [%s], using default", model.c_str());
		}
	}

	std::string ComputerPET2001::GetCharROMPath()
	{
		std::string path = m_basePathROM;
		switch (m_model)
		{
		case Model::BASIC1:
		case Model::BASIC1p:
		default:
			path.append("CHAR1.bin");
			break;
		case Model::BASIC2n:
		case Model::BASIC2p:
			path.append("CHAR2.bin");
			break;
		}

		LogPrintf(LOG_INFO, "Character ROM: %s", path.c_str());
		return path;
	}

	void ComputerPET2001::InitROM()
	{
		std::string c000 = m_basePathROM;
		std::string d000 = m_basePathROM;
		std::string e000 = m_basePathROM;
		std::string f000 = m_basePathROM;

		switch (m_model)
		{
		case Model::BASIC1:
		case Model::BASIC1p:
		default:
			c000.append((m_model == Model::BASIC1) ? "BASIC1-C000.bin" : "BASIC1-C000-p.bin");
			d000.append("BASIC1-D000.bin");
			e000.append("EDIT1-E000.bin");
			f000.append("KERNAL1-F000.bin");
			break;
		case Model::BASIC2n:
		case Model::BASIC2p:
			c000.append("BASIC2-C000.bin");
			d000.append("BASIC2-D000.bin");
			e000.append((m_model == Model::BASIC2n) ? "EDIT2-E000-n.bin" : "EDIT2-E000-p.bin");
			f000.append("KERNAL2-F000.bin");
			break;
		}

		LogPrintf(LOG_INFO, "ROM Set:");
		LogPrintf(LOG_INFO, "  C000: %s", c000.c_str());
		LogPrintf(LOG_INFO, "  D000: %s", d000.c_str());
		LogPrintf(LOG_INFO, "  E000: %s", e000.c_str());
		LogPrintf(LOG_INFO, "  F000: %s", f000.c_str());

		m_romC000.LoadFromFile(c000.c_str());
		m_romD000.LoadFromFile(d000.c_str());
		m_romE000.LoadFromFile(e000.c_str());
		m_romF000.LoadFromFile(f000.c_str());

		m_memory.Allocate(&m_romC000, 0xC000);
		m_memory.Allocate(&m_romD000, 0xD000);
		m_memory.Allocate(&m_romE000, 0xE000);
		m_memory.Allocate(&m_romF000, 0xF000);
	}

	void ComputerPET2001::InitVideo()
	{
		video::VideoPET2001* video = new video::VideoPET2001();
		video->EnableLog(CONFIG().GetLogLevel("video"));
		video->Init(&m_memory, GetCharROMPath().c_str());
		video->SetVIA(&m_via);
		m_video = video;

		m_memory.Allocate(&m_videoRAM, 0x8000);
		m_memory.MapWindow(0x8000, 0x8400, 0x0400);
		m_memory.MapWindow(0x8000, 0x8800, 0x0400);
		m_memory.MapWindow(0x8000, 0x8C00, 0x0400);
	}

	void ComputerPET2001::InitIO()
	{
		m_memory.Allocate(&m_ioE800, 0xE800);
		// TODO: No copy on board #4 to leave room at 0xE900-0xEFFF for nationalized keyboard mappings
		for (WORD b = 0xE900; b < 0xF000; b += 0x100)
		{
			m_memory.MapWindow(0xE800, b, 0x0100);
		}

		// PIA1 @ E8[10]
		m_pia1.EnableLog(CONFIG().GetLogLevel("pet.pia1"));
		m_pia1.Init(&m_keyboard);
		// Incomplete decoding, will also select at 3x, 5x, 7x etc
		m_ioE800.AddDevice(m_pia1, 0x10);

		// PIA2 @ E8[20]
		m_pia2.EnableLog(CONFIG().GetLogLevel("pet.pia2"));
		m_pia2.Init();
		// Incomplete decoding, will also select at 3x, 6x, 7x, Ax, Bx, etc
		m_ioE800.AddDevice(m_pia2, 0x20);

		// VIA @ E8[40]
		m_via.EnableLog(CONFIG().GetLogLevel("pet.via"));
		m_via.Init();
		// Incomplete decoding, will also select at 5x-7x, Cx-Fx
		m_ioE800.AddDevice(m_via, 0x40);
	}

	void ComputerPET2001::InitRAM(emul::WORD baseRAM)
	{
		LogPrintf(LOG_INFO, "Requested base RAM: %dKB", baseRAM);

		if (baseRAM < 4)
		{
			LogPrintf(LOG_WARNING, "Requested base RAM too low (%dKB), using 4KB", baseRAM);
			baseRAM = 4;
		}

		if (baseRAM > 32)
		{
			LogPrintf(LOG_WARNING, "Requested base RAM too high (%dKB), using 32KB", baseRAM);
			baseRAM = 32;
		}

		m_baseRAM.Alloc(baseRAM * 1024);
		m_memory.Clear();
		m_memory.Allocate(&m_baseRAM, 0);
	}

	void ComputerPET2001::InitTape()
	{
		m_tape.Init(2);
	}

	void ComputerPET2001::Reset()
	{
		ComputerBase::Reset();
		m_pia1.Reset();
		m_pia2.Reset();
		m_via.Reset();
		m_keyboard.Reset();
	}

	bool ComputerPET2001::Step()
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
				SOUND().PlayMono(m_via.GetCassetteDataOut() ? 16384 : 0);
			}

			// Tape update
			{
				TapeDeck& tape1 = m_tape.GetTape(0);
				TapeDeck& tape2 = m_tape.GetTape(1);

				m_pia1.SetCassette1ReadLine(tape1.Read());
				m_via.SetCassette2ReadLine(tape2.Read());
				tape1.Write(m_via.GetCassetteDataOut());
				tape2.Write(m_via.GetCassetteDataOut());
				tape1.SetMotor(m_pia1.GetCassette1MotorOut());
				tape2.SetMotor(m_via.GetCassette2MotorOut());
				m_pia1.SetCassetteSense1In(tape1.GetSense());
				m_pia1.SetCassetteSense2In(tape2.GetSense());

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
			if (blank != oldBlank)
			{
				// TODO: Invert?
				m_pia1.SetScreenRetrace(blank);

				m_via.SetRetraceIn(blank);
				oldBlank = blank;
			}

			m_via.Tick();

			// All IRQ lines are connected together (wire-OR)
			GetCPU().SetIRQ(
				m_pia1.GetIRQA() ||
				m_pia1.GetIRQB() ||
				m_pia2.GetIRQA() ||
				m_pia2.GetIRQB() ||
				m_via.GetIRQ());
		}

		return true;
	}

	void ComputerPET2001::Serialize(json& to)
	{
		ComputerBase::Serialize(to);

		m_pia1.Serialize(to["pia1"]);
		m_pia2.Serialize(to["pia2"]);
		m_via.Serialize(to["via"]);
	}

	void ComputerPET2001::Deserialize(const json& from)
	{
		ComputerBase::Deserialize(from);

		m_pia1.Deserialize(from["pia1"]);
		m_pia2.Deserialize(from["pia2"]);
		m_via.Deserialize(from["via"]);
	}
}
