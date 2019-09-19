#include "stdafx.h"
#include "Console.h"
#include "TMS1000.h"

namespace Console 
{
	typedef std::tuple<char*, DWORD> ResourceData;

	HANDLE m_hConsole;
	CPUInfo* m_pCPUInfo = nullptr;
	static const char hexDigits[] = "0123456789ABCDEF";

	ResourceData GetBinaryResource(DWORD resourceID) {
		HRSRC		res;
		HGLOBAL		res_handle = NULL;
		char *      res_data;
		DWORD       res_size;

		res = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceID), RT_RCDATA);
		if (!res)
			return std::make_tuple(nullptr, 0);
		res_handle = LoadResource(NULL, res);
		if (!res_handle)
			return std::make_tuple(nullptr, 0);

		res_data = (char*)LockResource(res_handle);
		res_size = SizeofResource(NULL, res);

		return std::make_tuple(res_data, res_size);
	}

	void Init(CPUInfo* pCPUInfo) {
		m_pCPUInfo = pCPUInfo;
		m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		SetConsoleScreenBufferSize(m_hConsole, { 80, 25 });
		SMALL_RECT windowSize = { 0, 0, 80, 25 };
		SetConsoleWindowInfo(m_hConsole, TRUE, &windowSize);	
		SetConsoleTitle("TMS1000");
		SetConsoleCursorPosition(m_hConsole, { 0, 0 });

		CONSOLE_FONT_INFOEX cfi = { sizeof(cfi) };
		// Populate cfi with the screen buffer's current font info
		GetCurrentConsoleFontEx(m_hConsole, FALSE, &cfi);

		// Modify the font size in cfi
		cfi.dwFontSize.X *= 2;
		cfi.dwFontSize.Y *= 2;

		// Use cfi to set the screen buffer's new font
		SetCurrentConsoleFontEx(m_hConsole, FALSE, &cfi);

		DWORD dwMode = 0;
		GetConsoleMode(m_hConsole, &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(m_hConsole, dwMode);

		ResourceData data = GetBinaryResource(m_pCPUInfo->GetResourceID());

		if (std::get<0>(data)) {
			DWORD written;
			WriteConsole(m_hConsole,
				std::get<0>(data),
				std::get<1>(data),
				&written,
				NULL);
		}
	}

	void WriteRegisterValueBin(WORD value, const CPUInfo::Coord& coord) {
		static char bits[17];
		bits[16] = 0;

		for (int i = 0; i < coord.w; ++i) {
			bits[16 - i] = (value & (1 << i)) ? '1' : '0';
		}

		SetConsoleCursorPosition(m_hConsole, { coord.x - 1, coord.y });
		DWORD written;
		WriteConsole(m_hConsole, bits + 17 - coord.w, coord.w, &written, NULL);
	}

	void WriteRegisterValueHex(WORD value, const CPUInfo::Coord& coord) {
		static char hex[3];
		hex[0] = hexDigits[(value & 0xF0) >> 4];
		hex[1] = hexDigits[(value & 0x0F)];
		hex[2] = 0;

		SetConsoleCursorPosition(m_hConsole, { coord.x - 1, coord.y });
		DWORD written;
		WriteConsole(m_hConsole, hex + 2 - coord.w, coord.w, &written, NULL);
	}

	void WriteRegisterValue(WORD value, const char* label) {
		CPUInfo::Coord coord = m_pCPUInfo->GetCoord(label);

		if (coord.IsSet()) {
			WriteRegisterValueBin(value, coord);
		}

		std::string hexLabel = label;
		hexLabel.append(".hex");
		coord = m_pCPUInfo->GetCoord(hexLabel.c_str());
		if (coord.IsSet()) {
			WriteRegisterValueHex(value, coord);
		}
	}

	void WriteRAM() {
		const short lineLen = 16;
		static const CPUInfo::Coord coord = m_pCPUInfo->GetCoord("RAM");
		static short lines = m_pCPUInfo->GetRAMWords() / lineLen;
		
		static char ramLine[lineLen];
		for (short y = 0; y < lines; ++y) {
			for (short x = 0; x < lineLen; ++x) {
				ramLine[x] = hexDigits[TMS1000::g_memory.RAM[y*lineLen + x] & 0x0F];
			}

			SetConsoleCursorPosition(m_hConsole, { coord.x - 1, coord.y + y });
			DWORD written;
			WriteConsole(m_hConsole, ramLine, lineLen, &written, NULL);
		}
	}

	void WriteROM() {
		const short lineLen = 16; // 16 words * 2 char/word
		static const CPUInfo::Coord coord = m_pCPUInfo->GetCoord("ROM");
		const short lines = 64 / lineLen;

		WORD baseAddr = TMS1000::g_cpu.PA * 64;

		static char romLine[lineLen * 2];
		for (short y = 0; y < lines; ++y) {
			for (short x = 0; x < lineLen; ++x) {
				romLine[x * 2] = hexDigits[(TMS1000::g_memory.ROM[y*lineLen + x + baseAddr] & 0xF0) >> 4];
				romLine[x * 2 + 1] = hexDigits[TMS1000::g_memory.ROM[y*lineLen + x + baseAddr] & 0x0F];
			}

			SetConsoleCursorPosition(m_hConsole, { coord.x - 1, coord.y + y });
			DWORD written;
			WriteConsole(m_hConsole, romLine, lineLen * 2, &written, NULL);
		}

		static const CPUInfo::Coord pageCoord = m_pCPUInfo->GetCoord("ROMPage");
		if (pageCoord.IsSet()) {
			SetConsoleCursorPosition(m_hConsole, { pageCoord.x - 1, pageCoord.y });
			DWORD written;
			romLine[0] = hexDigits[TMS1000::g_cpu.PA];
			WriteConsole(m_hConsole, romLine, 1, &written, NULL);
		}
	}

	void WriteDisassembly() {
		static const CPUInfo::Coord coordAddr = m_pCPUInfo->GetCoord("DAddr");
		static const CPUInfo::Coord coordData = m_pCPUInfo->GetCoord("DData");
		if (!coordData.IsSet() || !coordAddr.IsSet())
			return;

		static char line[16];

		WORD baseAddr = TMS1000::g_cpu.PA * 64;
		const int height = 12; // Heights are hardcoded for now
		for (short y = 0; y < height; ++y) {
			WORD PC = (TMS1000::g_cpu.PC -4 + y) & 0x3F;

			line[0] = hexDigits[((PC + baseAddr) & 0xF00) >> 8];
			line[1] = hexDigits[((PC + baseAddr) & 0x0F0) >> 4];
			line[2] = hexDigits[(PC + baseAddr) & 0x00F];
			
			SetConsoleCursorPosition(m_hConsole, { coordAddr.x - 1, coordAddr.y + y });
			DWORD written;
			WriteConsole(m_hConsole, line, 3, &written, NULL);

			BYTE opcode = TMS1000::g_memory.ROM[PC + baseAddr];
			TMS1000::Disassemble(opcode, line, sizeof(line));

			SetConsoleCursorPosition(m_hConsole, { coordData.x - 1, coordData.y + y });
			WriteConsole(m_hConsole, line, coordData.w, &written, NULL);
		}

	}

	void UpdateStatus() {
		WriteRegisterValue(TMS1000::g_cpu.X, "X");
		WriteRegisterValue(TMS1000::g_cpu.Y, "Y");
		WriteRegisterValue(TMS1000::g_cpu.A, "A");
	
		WriteRegisterValue(TMS1000::g_cpu.S, "S");
		WriteRegisterValue(TMS1000::g_cpu.SL, "SL");

		WriteRegisterValue(TMS1000::g_cpu.O, "O");
		WriteRegisterValue(TMS1000::g_cpu.R, "R");
		WriteRegisterValue(TMS1000::g_cpu.K, "K");

		WriteRegisterValue(TMS1000::g_cpu.PA, "PA");
		WriteRegisterValue(TMS1000::g_cpu.PB, "PB");
		WriteRegisterValue(TMS1000::g_cpu.PC, "PC");
		WriteRegisterValue(TMS1000::g_cpu.SR, "SR");
		WriteRegisterValue(TMS1000::g_cpu.CL, "CL");

		WriteRAM();
		WriteROM();
		WriteDisassembly();
	}
}