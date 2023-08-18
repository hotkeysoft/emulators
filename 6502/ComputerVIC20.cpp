#include "stdafx.h"

#include "ComputerVIC20.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU6502.h"
#include <Sound/Sound.h>
#include "IO/DeviceKeyboardVIC20.h"
#include "Video/VideoVIC.h"
#include <FileUtil.h>

using cfg::CONFIG;
using sound::SOUND;
using tape::TapeDeck;
using video::vic::VideoVIC;
using sound::vic::SoundVIC;
using hscommon::fileUtil::File;

namespace emul
{
	const size_t MAIN_CLK_NTSC = 14318180;
	const size_t MAIN_CLK_PAL = 4433618;

	const size_t PIXEL_CLK_NTSC = (MAIN_CLK_NTSC / 7) * 2;
	const size_t PIXEL_CLK_PAL = MAIN_CLK_PAL;

	const size_t CPU_CLK_NTSC = PIXEL_CLK_NTSC / 4;
	const size_t CPU_CLK_PAL = PIXEL_CLK_PAL / 4;

	const size_t VIC_COLUMNS_NTSC = 65;
	const size_t VIC_COLUMNS_PAL = 71;

	const size_t VIC_ROWS_NTSC = 261;
	const size_t VIC_ROWS_PAL = 312;

	// Poll each frame
	const size_t SCAN_RATE_NTSC = VIC_COLUMNS_NTSC * VIC_ROWS_NTSC;
	const size_t SCAN_RATE_PAL = VIC_COLUMNS_PAL * VIC_ROWS_PAL;

	const char* GetMemoryLayoutStr(ComputerVIC20::MemoryLayout layout)
	{
		switch (layout)
		{
		case ComputerVIC20::MemoryLayout::UNKNOWN: return "UNKNOWN";
		case ComputerVIC20::MemoryLayout::MEM_5K:  return "5K";
		case ComputerVIC20::MemoryLayout::MEM_8K:  return "8K";
		case ComputerVIC20::MemoryLayout::MEM_16K: return "16K";
		case ComputerVIC20::MemoryLayout::MEM_24K: return "24K";
		case ComputerVIC20::MemoryLayout::MEM_32K: return "32K";
		default: throw std::exception("invalid memory layout");
		}
	}

	const char* ComputerVIC20::GetModelStr(ComputerVIC20::Model model)
	{
		switch (model)
		{
		case ComputerVIC20::Model::NTSC: return "ntsc";
		case ComputerVIC20::Model::PAL: return "pal";
		default: throw std::exception("invalid model");
		}
	}

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
		m_romBlock1("ROM-BLK1", 0x2000, emul::MemoryType::ROM),
		m_romBlock2("ROM-BLK2", 0x2000, emul::MemoryType::ROM),
		m_romBlock3("ROM-BLK3", 0x2000, emul::MemoryType::ROM),
		m_romBlock5("ROM-BLK5", 0x2000, emul::MemoryType::ROM),
		m_romBASIC("ROM-BASIC", 0x2000, emul::MemoryType::ROM),
		m_romKERNAL("ROM-KERNAL", 0x2000, emul::MemoryType::ROM),
		m_ioVIC("IOVIC", 0x0100),
		m_ioVIA("IOVIA", 0x0300),
		m_via1("VIA1"),
		m_via2("VIA2")

	{
	}

	ComputerVIC20::~ComputerVIC20()
	{
		delete m_keyboard;
		delete m_tape;
	}

	void ComputerVIC20::Init(WORD baseRAM)
	{
		ComputerBase::Init(CPUID_6502, baseRAM);

		InitModel();
		InitKeyboard();
		InitJoystick();
		InitRAM();
		InitROM();
		InitIO();
		InitSound();
		InitVideo();
		InitTape();

		if (m_model == Model::NTSC)
		{
			InitInputs(CPU_CLK_NTSC, SCAN_RATE_NTSC);
			SOUND().SetBaseClock(CPU_CLK_NTSC);
		}
		else
		{
			InitInputs(CPU_CLK_PAL, SCAN_RATE_PAL);
			SOUND().SetBaseClock(CPU_CLK_PAL);
		}

		GetInputs().InitKeyboard(m_keyboard);
		GetInputs().InitJoystick(&m_joystick);
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

	void ComputerVIC20::InitModel()
	{
		m_model = Model::NTSC;
		std::string modelStr = CONFIG().GetValueStr("core", "model", "ntsc");

		if (modelStr == GetModelStr(Model::NTSC))
		{
			m_model = Model::NTSC;
		}
		else if (modelStr == GetModelStr(Model::PAL))
		{
			m_model = Model::PAL;
		}
		else
		{
			LogPrintf(LOG_WARNING, "Unknown model [%s], using default", modelStr.c_str());
		}

		LogPrintf(LOG_INFO, "Model: %s", GetModelStr(m_model));
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

		if (m_model == Model::NTSC)
		{
			m_romKERNAL.LoadFromFile("data/VIC20/KERNAL-E000-ntsc.bin");
		}
		else
		{
			m_romKERNAL.LoadFromFile("data/VIC20/KERNAL-E000-pal.bin");
		}
		m_memory.Allocate(&m_romKERNAL, 0xE000);
	}

	void ComputerVIC20::InitVideo()
	{
		VideoVIC* video = new VideoVIC(
			m_sound,
			(m_model == Model::NTSC) ? VIC_COLUMNS_NTSC : VIC_COLUMNS_PAL,
			(m_model == Model::NTSC) ? VIC_ROWS_NTSC : VIC_ROWS_PAL);
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

	void ComputerVIC20::InitRAM()
	{
		LogPrintf(LOG_INFO, "Requested base RAM: %dKB", m_baseRAMSize);

		WORD actualRAM = m_baseRAMSize;

		if (actualRAM < 5)
		{
			LogPrintf(LOG_WARNING, "Requested base RAM too low (%dKB), using 4KB", actualRAM);
			actualRAM = 5;
		}

		// Above 8K, Round to 8k block
		if (actualRAM > 5)
		{
			WORD rounded = RoundPowerOf2(actualRAM, 8);
			if (rounded != actualRAM)
			{
				actualRAM = rounded;
				LogPrintf(LOG_WARNING, "Requested base RAM rounded to (%dKB)", actualRAM);
			}
		}

		if (actualRAM > 32)
		{
			actualRAM = 32;
			LogPrintf(LOG_WARNING, "Requested base RAM too high (%dKB), using 32KB", actualRAM);
		}

		m_baseRAMSize = actualRAM;

		ResetMemoryLayout();
		switch (actualRAM)
		{
		case 32:
			m_memory.Allocate(&m_ramBlock1, 0x6000);
			SetMemoryLayout(MemoryLayout::MEM_32K);
			[[fallthrough]];
		case 24:
			m_memory.Allocate(&m_ramBlock1, 0x4000);
			SetMemoryLayout(MemoryLayout::MEM_24K);
			[[fallthrough]];
		case 16:
			m_memory.Allocate(&m_ramBlock1, 0x2000);
			SetMemoryLayout(MemoryLayout::MEM_16K);
			[[fallthrough]];
		case 8:
			m_memory.Allocate(&m_ramBlock0RAM3, 0x0C00);
			m_memory.Allocate(&m_ramBlock0RAM2, 0x0800);
			m_memory.Allocate(&m_ramBlock0RAM1, 0x0400);
			SetMemoryLayout(MemoryLayout::MEM_8K);
			[[fallthrough]];
		default:
			// Base 5K: Always allocated
			m_memory.Allocate(&m_ramBlock0LOW, 0x0000);
			m_memory.Allocate(&m_ramBlock0MAIN, 0x1000);
			SetMemoryLayout(MemoryLayout::MEM_5K);
		}

		m_memory.Allocate(&m_ramCOLOR, 0x9400);
		m_memory.Clear();
	}

	void ComputerVIC20::InitTape()
	{
		m_tape = new tape::DeviceTape((m_model == Model::NTSC) ? CPU_CLK_NTSC : CPU_CLK_PAL);
		m_tape->Init(1);
	}

	void ComputerVIC20::Reset()
	{
		ComputerBase::Reset();
		m_via1.Reset();
		m_via2.Reset();
		m_keyboard->Reset();
		m_tape->Reset();
	}

	void ComputerVIC20::ResetMemoryLayout()
	{
		m_memoryLayout = MemoryLayout::UNKNOWN;
	}

	void ComputerVIC20::SetMemoryLayout(MemoryLayout layout)
	{
		if (m_memoryLayout == MemoryLayout::UNKNOWN)
		{
			m_memoryLayout = layout;
			LogPrintf(LOG_INFO, "Memory Configuration: %s", GetMemoryLayoutStr(m_memoryLayout));
		}
		else
		{
			// Ignore calls after already set
		}
	}

	bool ComputerVIC20::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		uint32_t cpuTicks = GetCPU().GetInstructionTicks();

		TapeDeck& tape = m_tape->GetTape(0);

		for (uint32_t i = 0; i < cpuTicks; ++i)
		{
			++g_ticks;

			if (!m_turbo)
			{
				WORD sound = m_sound.GetOutput() + tape.GetSound();
				SOUND().PlayMono(sound);
			}

			// Tape update
			{
				m_via2.SetCassetteDataIn(tape.Read());
				tape.Write(m_via2.GetCassetteDataOut());
				tape.SetMotor(m_via1.GetCassetteMotorOut());
				m_via1.SetCassetteSwitchIn(tape.GetSense());

				m_tape->Tick();
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

	bool ComputerVIC20::ValidateRAMBlock(ADDRESS loadAddress, const MemoryBlock::RawBlock& block)
	{
		MemoryLayout toLoadLayout = MemoryLayout::UNKNOWN;

		switch (loadAddress)
		{
		case 0x1001:
			LogPrintf(LOG_INFO, "LoadPRG: Detected BASIC program (5K Config)");
			toLoadLayout = MemoryLayout::MEM_5K;
			break;
		case 0x0401:
			LogPrintf(LOG_INFO, "LoadPRG: Detected BASIC program (8K Config)");
			toLoadLayout = MemoryLayout::MEM_8K;
			break;
		case 0x1201:
			LogPrintf(LOG_INFO, "LoadPRG: Detected BASIC program (>=16K Config)");
			toLoadLayout = MemoryLayout::MEM_16K;
			break;
		default:
			LogPrintf(LOG_INFO, "LoadPRG: Unknown configuration, loading at address %04X", loadAddress);
			toLoadLayout = MemoryLayout::UNKNOWN;
			return true; // Don't do any validation
		}

		// Check for mismatch with current configuration
		if (GetMemoryLayout() == toLoadLayout)
		{
			return true;
		}

		// Mismatch... possibly
		// 16K expected should match 16, 24 or 32.
		// We don't know if the PRG expects 24 or 32, we can only check the minimum
		if ((toLoadLayout == MemoryLayout::MEM_16K) && (GetMemoryLayout() >= toLoadLayout))
		{
			return true;
		}

		// Real mismatch
		LogPrintf(LOG_ERROR, "Incompatible RAM Configuration. PRG Expects=[%s], Actual=[%s]",
			GetMemoryLayoutStr(toLoadLayout),
			GetMemoryLayoutStr(GetMemoryLayout()));

		return false;
	}

	void ComputerVIC20::LoadRAMBlock(ADDRESS loadAddress, const MemoryBlock::RawBlock& block)
	{
		if (!m_memory.FillRAM(loadAddress, block))
		{
			LogPrintf(LOG_ERROR, "LoadRAMBlock failed, Reset computer");
			Reset();
			return;
		}

		ADDRESS end = loadAddress + (WORD)block.size();

		ADDRESS basicStart = m_memory.Read16(0x2B);

		if (loadAddress == basicStart)
		{
			LogPrintf(LOG_INFO, "LoadRAMBlock: Adjusting BASIC pointers");

			m_memory.Write16(0x2D, end);
			m_memory.Write16(0x2F, end + 6);
			m_memory.Write16(0x31, end + 6);
		}
	}

	void ComputerVIC20::LoadROMBlock(ADDRESS loadAddress, const MemoryBlock::RawBlock& block)
	{
		MemoryBlock* target = nullptr;
		switch (loadAddress)
		{
		case 0x2000: target = &m_romBlock1; break;
		case 0x4000: target = &m_romBlock2; break;
		case 0x6000: target = &m_romBlock3; break;
		case 0xA000: target = &m_romBlock5; break;
		default: target = nullptr; break;
		}

		if (!target)
		{
			LogPrintf(LOG_INFO, "LoadROMBlock: Invalid load address %04X", loadAddress);
			return;
		}

		target->Fill(0, block);
		m_memory.Allocate(target, loadAddress);

		if (loadAddress == 0xA000)
		{
			Reset();
		}
	}

	// Unload all ROM blocks, put back RAM block (if they were present)
	void ComputerVIC20::UnloadPRG()
	{
		m_memory.Free(&m_romBlock1);
		m_memory.Free(&m_romBlock2);
		m_memory.Free(&m_romBlock3);
		m_memory.Free(&m_romBlock5);
		InitRAM();
		Reset();
	}

	void ComputerVIC20::LoadPRG(const hscommon::fileUtil::PathList& paths)
	{
		if (paths.size() == 0)
		{
			LogPrintf(LOG_WARNING, "LoadPRG: No files selected");
			return;
		}

		for (auto& path : paths)
		{
			std::string pathStr = path.string();
			LogPrintf(LOG_INFO, "LoadPRG: loading %s", pathStr.c_str());

			File f(pathStr.c_str(), "rb");
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

			switch (loadAddress)
			{
			case 0x2000:
			case 0x4000:
			case 0x6000:
			case 0xA000:
				LoadROMBlock(loadAddress, buf);
				break;
			default:
				if (ValidateRAMBlock(loadAddress, buf))
				{
					LoadRAMBlock(loadAddress, buf);
				}
				break;
			}
		}
	}

	void ComputerVIC20::Serialize(json& to)
	{
		ComputerBase::Serialize(to);

		to["model"] = GetModelStr(m_model);

		m_via1.Serialize(to["via1"]);
		m_via2.Serialize(to["via2"]);
	}

	void ComputerVIC20::Deserialize(const json& from)
	{
		ComputerBase::Deserialize(from);

		if (GetModelStr(m_model) != from["model"])
		{
			throw SerializableException("ComputerVIC20: Model is not compatible", SerializationError::COMPAT);
		}

		m_via1.Deserialize(from["via1"]);
		m_via2.Deserialize(from["via2"]);
	}
}
