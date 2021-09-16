#include "Computer.h"

#include "Console.h"

namespace emul
{

	enum SCREENWIDTH { COLS40 = 40, COLS80 = 80 };
	const SCREENWIDTH screenWidth = COLS80;

	Computer::Computer() : 
		m_memory(emul::CPU8086_ADDRESS_BITS),
		m_base64K("RAM0", 0x10000, emul::MemoryType::RAM),
		Logger("PC"), 
		CPU8086(m_memory, m_map),
		m_pit(0x40, 1193182),
		m_pic(0x20),
		m_ppi(0x60),
		m_dma(0x00, m_memory),
		m_cga(0x3D0),
		m_floppy(0x03F0, 1193182)
	{
	}

	void Computer::Init()
	{
		m_memory.EnableLog(true, Logger::LOG_INFO);
		m_mmap.EnableLog(true, Logger::LOG_INFO);

		m_base64K.Clear(0xA5);
		m_memory.Allocate(&m_base64K, 0);

		m_pit.Init();
		m_pit.EnableLog(true, Logger::LOG_WARNING);

		m_pic.Init();
		m_pic.EnableLog(true, Logger::LOG_WARNING);

		m_ppi.Init();
		m_ppi.EnableLog(true, Logger::LOG_WARNING);

		// Configuration switches
		{
			m_ppi.SetPOSTLoop(false);
			m_ppi.SetMathCoprocessor(false);
			m_ppi.SetRAMConfig(ppi::RAMSIZE::RAM_64K);
			m_ppi.SetDisplayConfig(screenWidth == COLS80 ? ppi::DISPLAY::COLOR_80x25 : ppi::DISPLAY::COLOR_40x25);
			m_ppi.SetFloppyCount(2);
		}

		m_dma.Init();
		m_dma.EnableLog(false);
		m_dma.EnableLog(true, Logger::LOG_INFO);

		m_cga.EnableLog(true, Logger::LOG_INFO);
		m_cga.Init("data/CGA_CHAR.BIN");
		m_memory.Allocate(&m_cga.GetVideoRAM(), emul::S2A(0xB800));
		m_memory.Allocate(&m_cga.GetVideoRAM(), emul::S2A(0xBC00));

		m_floppy.Init();
		m_floppy.EnableLog(true, Logger::LOG_INFO);
		m_floppy.LoadDiskImage(0, "data/PC-DOS-1.00.img");
		m_floppy.LoadDiskImage(1, "data/PCMAG-VOL06N19.img");

		AddDevice(m_pic);
		AddDevice(m_pit);
		AddDevice(m_ppi);
		AddDevice(m_dma);
		AddDevice(m_cga);
		AddDevice(m_floppy);
	}

	bool Computer::Step()
	{
		++m_ticks; // TODO: Coordinate with CPU

		// TODO: Temporary, need dynamic connections
		// Timer 0: Time of day
		// Timer 1: DMA RAM refresh
		// Timer 2: Speaker
		static bool timer0Out = false;

		if (!CPU8086::Step())
		{
			return false;
		}

		if (m_ticks % 10000 == 0)
		{
			if (m_keyBufRead != m_keyBufWrite && CanInterrupt())
			{
				Interrupt(8 + 1); // Hardware interrupt 1: keyboard
				m_ppi.SetCurrentKeyCode(m_keyBuf[m_keyBufRead++]);
			}
		}

		m_pit.Tick();
		bool out = m_pit.GetCounter(0).GetOutput();
		if (out != timer0Out)
		{
			timer0Out = out;
			if (out)
			{
				// TODO: this bypasses a lot of things.
				// Quick and dirty for now: Check mask manually and interrupt cpu
				if (!(m_pic.Mask_IN() & 0x01))
				{
					Interrupt(8 + 6); // Hardware interrupt 0: timer
				}
			}
		}

		if (m_floppy.IsInterruptPending() && CanInterrupt())
		{
			//fprintf(stderr, "Fire Floppy interrupt\n");
			Interrupt(8 + 6); // Hardware interrupt 6: floppy
			m_floppy.ClearInterrupt();
		}

		m_dma.Tick();
		m_cga.Tick();
		m_floppy.Tick();

		// TODO: faking it
		if (m_floppy.IsDMAPending())
		{
			m_dma.DMARequest(2, true);
			//fprintf(stderr, "floppy DMA pending\n");
		}

		if (m_dma.DMAAcknowledged(2))
		{
			m_dma.DMARequest(2, false);

			// Do it manually
			m_floppy.DMAAcknowledge();
			m_dma.DMAWrite(2, m_floppy.ReadDataFIFO());
			if (m_dma.GetTerminalCount(2))
			{
				m_floppy.DMATerminalCount();
			}
			//fprintf(stderr, "floppy DMA read\n");
		}

		return true;
	}
}
