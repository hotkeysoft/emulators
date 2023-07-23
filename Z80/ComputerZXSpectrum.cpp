#include "stdafx.h"

#include "ComputerZXSpectrum.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPUZ80.h"
#include "Video/VideoZXSpectrum.h"
#include <Sound/Sound.h>

using cfg::CONFIG;
using sound::SOUND;

namespace emul
{
	// TODO
	const size_t MAIN_CLK = 14000000; // 14 MHz Main crystal
	const size_t PIXEL_CLK = MAIN_CLK / 2;
	const size_t CPU_CLK = PIXEL_CLK / 2;

	// TODO: ULA
	// Every 20ms, generate interrupt
	// For now base on CPU clock
	const size_t RTC_CLK = 50; // 50Hz
	const size_t RTC_RATE = CPU_CLK / RTC_CLK;


	ComputerZXSpectrum::ComputerZXSpectrum() :
		Logger("ZXSpectrum"),
		ComputerBase(m_memory),
		m_baseRAM("RAM", 0x4000, emul::MemoryType::RAM),
		m_rom("ROM", 0x4000, emul::MemoryType::ROM)
	{
	}

	void ComputerZXSpectrum::Init(WORD baseRAM)
	{
		PortConnector::Init(PortConnectorMode::BYTE_LOW);
		ComputerBase::Init(CPUID_Z80, baseRAM);

		EnableLog(LOG_DEBUG);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_rom.LoadFromFile("data/z80/zxspectrum48.bin");
		m_memory.Allocate(&m_rom, 0);

		m_memory.Allocate(&m_baseRAM, 0x4000);
		//m_memory.MapWindow(0x4000, 0xC000, 0x4000);

		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZXSpectrum::ReadKeyboard));

		// Officially 0xFE, but in fact all even ports
		for (int i = 0; i < 128; ++i)
		{
			Connect(i << 1, static_cast<PortConnector::OUTFunction>(&ComputerZXSpectrum::WriteULA));
		}

		InitInputs(CPU_CLK, RTC_RATE);
		GetInputs().InitKeyboard(&m_keyboard);

		SOUND().SetBaseClock(CPU_CLK);
		InitVideo();
	}

	void ComputerZXSpectrum::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_Z80) m_cpu = new CPUZ80(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void ComputerZXSpectrum::InitVideo()
	{
		video::VideoZXSpectrum* video = new video::VideoZXSpectrum();

		m_video = video;
		m_video->Init(&m_memory, nullptr);
	}

	bool ComputerZXSpectrum::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		uint32_t cpuTicks = GetCPU().GetInstructionTicks();

		for (uint32_t i = 0; i < cpuTicks; ++i)
		{
			++g_ticks;

			GetVideo().Tick();
			GetVideo().Tick();

			// Generate an interrupt every RTC_CLK (50Hz)
			static size_t rtcDelay = RTC_RATE;
			GetCPU().SetINT(rtcDelay == RTC_RATE);
			if (--rtcDelay == 0)
			{
				GetVideo().VSync();

				// Reset Counter
				rtcDelay = RTC_RATE;
			}

			if (!m_turbo)
			{
				SOUND().PlayMono(m_earOutput << 8);
			}

			GetInputs().Tick();
			if (GetInputs().IsQuit())
			{
				return false;
			}
		}

		return true;
	}

	void ComputerZXSpectrum::WriteULA(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteULA, value = %02X", value);
		GetVideo().SetBorder(value & 0b111);

		m_earOutput = GetBit(value, 4);
		m_micOutput = GetBit(value, 3);
	}

	BYTE ComputerZXSpectrum::ReadKeyboard()
	{
		BYTE row = GetHByte(GetCurrentPort());
		LogPrintf(LOG_DEBUG, "ReadKeyboard, row = %02X", row);

		BYTE data = ~m_keyboard.GetRowData(row);

		return data;
	}
}
