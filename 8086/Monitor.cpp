#include "Monitor.h"

#include <string>

namespace emul
{
	static const char hexDigits[] = "0123456789ABCDEF";

	Monitor::Monitor(Console& console) :
		m_console(console)
	{
	}

	void Monitor::Init(CPU8086& cpu, Memory& memory)
	{
		m_cpu = &cpu;
		m_memory = &memory;

		try
		{
			g_CPUInfo.LoadConfig();
		}
		catch (nlohmann::detail::exception e)
		{
			fprintf(stderr, "Error loading config: %s\n", e.what());
			throw;
		}
	}

	void Monitor::SendKey(char ch)
	{
		switch (ch)
		{
		case 63: // F5
			ToggleRunMode();
			break;

		case 62: // F4
			ToggleRAMMode();

			// Not implemented, ignore
		case 59: // F1
		case 60: // F2
		case 61: // F3
		case 64: // F6
		case 65: // F7
		case 66: // F8
		case 67: // F9
		case 68: // F10
		default:
			if (m_runMode == RUNMode::STEP)
				m_console.WaitForKey();
			break;
		}
	}

	void Monitor::Show()
	{
		std::string ansiFile = g_CPUInfo.GetANSIFile();

		if (ansiFile.size()) 
		{
			m_console.WriteBuffer(ansiFile.c_str(), ansiFile.size());
		}

		Update();
		UpdateRunMode();
		UpdateRAMMode();
	}

	void Monitor::WriteValueHex(BYTE value, const CPUInfo::Coord& coord, WORD attr)
	{
		static char hex[2];
		hex[0] = hexDigits[value >> 4];
		hex[1] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 2, attr);
	}

	void Monitor::WriteValueHex(WORD value, const CPUInfo::Coord& coord, WORD attr)
	{
		static char hex[4];
		hex[0] = hexDigits[(value >> 12) & 0x0F];
		hex[1] = hexDigits[(value >> 8) & 0x0F];
		hex[2] = hexDigits[(value >> 4) & 0x0F];
		hex[3] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 4, attr);
	}

	void Monitor::Update()
	{
		UpdateRegisters();
		UpdateFlags();
		UpdateRAM();
		UpdateCode();
	}

	void Monitor::ToggleRunMode()
	{
		if (m_runMode == RUNMode::RUN) m_runMode = RUNMode::STEP;
		else m_runMode = RUNMode::RUN;

		UpdateRunMode();
	}

	void Monitor::UpdateRunMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (1 << 4) | 14;
		static CPUInfo::Coord run = g_CPUInfo.GetCoord("status.RUN");
		static CPUInfo::Coord stop = g_CPUInfo.GetCoord("status.STOP");

		m_console.WriteAttrAt(run.x, run.y, (m_runMode == RUNMode::RUN) ? highlight : regular, run.w);
		m_console.WriteAttrAt(stop.x, stop.y, (m_runMode == RUNMode::STEP) ? highlight : regular, stop.w);
	}

	void Monitor::ToggleRAMMode()
	{
		switch (m_ramMode)
		{
		case RAMMode::DSSI: m_ramMode = RAMMode::ESDI; break;
		case RAMMode::ESDI: m_ramMode = RAMMode::STACK; break;
		case RAMMode::STACK: m_ramMode = RAMMode::CUSTOM; break;
		case RAMMode::CUSTOM: m_ramMode = RAMMode::DSSI; break;
		}

		UpdateRAMMode();
	}

	void Monitor::UpdateRAMMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (0 << 4) | 8;
		static CPUInfo::Coord ramDSSI = g_CPUInfo.GetCoord("ram.DSSI");
		static CPUInfo::Coord ramESDI = g_CPUInfo.GetCoord("ram.ESDI");
		static CPUInfo::Coord ramSTACK = g_CPUInfo.GetCoord("ram.STACK");
		static CPUInfo::Coord ramCustom = g_CPUInfo.GetCoord("ram.CUSTOM");

		m_console.WriteAttrAt(ramDSSI.x, ramDSSI.y, (m_ramMode == RAMMode::DSSI) ? highlight : regular, ramDSSI.w);
		m_console.WriteAttrAt(ramESDI.x, ramESDI.y, (m_ramMode == RAMMode::ESDI) ? highlight : regular, ramESDI.w);
		m_console.WriteAttrAt(ramSTACK.x, ramSTACK.y, (m_ramMode == RAMMode::STACK) ? highlight : regular, ramSTACK.w);
		m_console.WriteAttrAt(ramCustom.x, ramCustom.y, (m_ramMode == RAMMode::CUSTOM) ? highlight : regular, ramCustom.w);
	}

	void Monitor::Step()
	{
		switch (m_runMode)
		{
		case RUNMode::RUN:
			break;
		case RUNMode::STEP:
			m_console.WaitForKey();
			break;
		}
	}

	void Monitor::UpdateRegisters()
	{
		WriteValueHex(m_cpu->regA.hl.h, g_CPUInfo.GetCoord("AH"));
		WriteValueHex(m_cpu->regA.hl.l, g_CPUInfo.GetCoord("AL"));

		WriteValueHex(m_cpu->regB.hl.h, g_CPUInfo.GetCoord("BH"));
		WriteValueHex(m_cpu->regB.hl.l, g_CPUInfo.GetCoord("BL"));

		WriteValueHex(m_cpu->regC.hl.h, g_CPUInfo.GetCoord("CH"));
		WriteValueHex(m_cpu->regC.hl.l, g_CPUInfo.GetCoord("CL"));

		WriteValueHex(m_cpu->regD.hl.h, g_CPUInfo.GetCoord("DH"));
		WriteValueHex(m_cpu->regD.hl.l, g_CPUInfo.GetCoord("DL"));

		WriteValueHex(m_cpu->regDS, g_CPUInfo.GetCoord("DS"));
		WriteValueHex(m_cpu->regSI, g_CPUInfo.GetCoord("SI"));

		WriteValueHex(m_cpu->regES, g_CPUInfo.GetCoord("ES"));
		WriteValueHex(m_cpu->regDI, g_CPUInfo.GetCoord("DI"));

		WriteValueHex(m_cpu->regBP, g_CPUInfo.GetCoord("BP"));

		WriteValueHex(m_cpu->regCS, g_CPUInfo.GetCoord("CS"));
		WriteValueHex(m_cpu->regIP, g_CPUInfo.GetCoord("IP"));

		WriteValueHex(m_cpu->regSS, g_CPUInfo.GetCoord("SS"));
		WriteValueHex(m_cpu->regSP, g_CPUInfo.GetCoord("SP"));
	}

	void Monitor::UpdateFlags()
	{
		static WORD attr[12];

		for (int i = 0; i < 12; ++i)
		{
			attr[i] = (m_cpu->flags & (1 << i)) ? 15 : 8;
		}

		static CPUInfo::Coord coord = g_CPUInfo.GetCoord("FLAGS");

		m_console.WriteAttrAt(coord.x, coord.y, attr, 12);
	}

	void Monitor::UpdateRAM()
	{
		static CPUInfo::Coord addrPos = g_CPUInfo.GetCoord("ram.ADDR");
		static CPUInfo::Coord hexPos = g_CPUInfo.GetCoord("ram.HEX");
		static CPUInfo::Coord charPos = g_CPUInfo.GetCoord("ram.CHAR");
		int bytesPerLine = charPos.w;
		hexPos.h = 8;

		//static char ramLine[80] = "                              ";

		BYTE* data = m_memory->GetPtr8(S2A(m_cpu->regDS, m_cpu->regSI));
		for (int i = 0; i < hexPos.h; ++i)
		{
			CPUInfo::Coord pos;
			pos.x = addrPos.x;
			pos.y = addrPos.y + i;
			WriteValueHex(m_cpu->regDS, pos);
			pos.x += 5;
			WriteValueHex((WORD)(m_cpu->regSI + (bytesPerLine * i)), pos);

			for (int x = 0; x < bytesPerLine; ++x)
			{
				pos.x = hexPos.x + (3 * x);
				WriteValueHex((BYTE)data[bytesPerLine * i + x], pos);
			}

			m_console.WriteAt(charPos.x, charPos.y + i, (const char*)data[bytesPerLine*i], bytesPerLine);
		}
	}

	void Monitor::UpdateCode()
	{
	}

}
