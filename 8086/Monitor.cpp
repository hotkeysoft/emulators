#include "Monitor.h"

#include <string>

namespace emul
{
	static const char hexDigits[] = "0123456789ABCDEF";

	Monitor::Monitor(Console& console) :
		m_console(console),
		m_cpu(nullptr),
		m_memory(nullptr)
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
		case 59: // F1
		case 60: // F2
		case 61: // F3
		case 62: // F4
		case 63: // F5
		case 64: // F6
		case 65: // F7
		case 66: // F8
		case 67: // F9
		case 68: // F10
		default:
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
	}

	void Monitor::WriteValueHex(BYTE value, const CPUInfo::Coord& coord, WORD attr)
	{
		static char hex[3];
		hex[0] = hexDigits[value >> 4];
		hex[1] = hexDigits[(value & 0x0F)];
		hex[2] = 0;

		m_console.WriteAt(coord.x, coord.y, hex + 2 - coord.w, coord.w, attr);
	}

	void Monitor::WriteValueHex(WORD value, const CPUInfo::Coord& coord, WORD attr)
	{
		static char hex[5];
		hex[0] = hexDigits[(value >> 12) & 0x0F];
		hex[1] = hexDigits[(value >> 8) & 0x0F];
		hex[2] = hexDigits[(value >> 4) & 0x0F];
		hex[3] = hexDigits[(value & 0x0F)];
		hex[4] = 0;

		m_console.WriteAt(coord.x, coord.y, hex + 4 - coord.w, coord.w, attr);
	}

	void Monitor::Update()
	{
		UpdateRegisters();
		UpdateRAM();
		UpdateCode();
	}

	void Monitor::UpdateRegisters()
	{
		WriteValueHex((BYTE)0, g_CPUInfo.GetCoord("AH"));
		WriteValueHex((BYTE)0, g_CPUInfo.GetCoord("AL"));

		WriteValueHex((BYTE)0, g_CPUInfo.GetCoord("BH"));
		WriteValueHex((BYTE)0, g_CPUInfo.GetCoord("BL"));

		WriteValueHex((BYTE)0, g_CPUInfo.GetCoord("CH"));
		WriteValueHex((BYTE)0, g_CPUInfo.GetCoord("CL"));

		WriteValueHex((BYTE)0, g_CPUInfo.GetCoord("DH"));
		WriteValueHex((BYTE)0, g_CPUInfo.GetCoord("DL"));

		WriteValueHex((WORD)0, g_CPUInfo.GetCoord("DS"));
		WriteValueHex((WORD)0, g_CPUInfo.GetCoord("SI"));

		WriteValueHex((WORD)0, g_CPUInfo.GetCoord("ES"));
		WriteValueHex((WORD)0, g_CPUInfo.GetCoord("DI"));

		WriteValueHex((WORD)0, g_CPUInfo.GetCoord("BP"));

		WriteValueHex((WORD)0, g_CPUInfo.GetCoord("CS"));
		WriteValueHex((WORD)0, g_CPUInfo.GetCoord("IP"));

		WriteValueHex((WORD)0, g_CPUInfo.GetCoord("SS"));
		WriteValueHex((WORD)0, g_CPUInfo.GetCoord("SP"));
	}

	void Monitor::UpdateRAM()
	{
	}

	void Monitor::UpdateCode()
	{
	}

}
