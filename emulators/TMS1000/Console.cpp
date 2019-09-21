#include "stdafx.h"
#include "Console.h"
#include "TMS1000.h"
#include <conio.h>

namespace Console 
{
	long lastTicks = 0;
	bool running = false;
	HANDLE m_hConsole;
	CPUInfo* m_pCPUInfo = nullptr;
	static const char hexDigits[] = "0123456789ABCDEF";

	void Init(CPUInfo* pCPUInfo) {
		m_pCPUInfo = pCPUInfo;
		m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		SetConsoleScreenBufferSize(m_hConsole, { 80, 25 });
		SMALL_RECT windowSize = { 0, 0, 80, 25 };
		SetConsoleWindowInfo(m_hConsole, TRUE, &windowSize);	
		SetConsoleTitle("TMS1000");
		SetConsoleCursorPosition(m_hConsole, { 0, 0 });

		CONSOLE_CURSOR_INFO cursorInfo = {1, FALSE };
		SetConsoleCursorInfo(m_hConsole, &cursorInfo);

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

		std::string ansiFile = m_pCPUInfo->GetANSIFile();

		if (ansiFile.size()) {
			DWORD written;
			WriteConsole(m_hConsole,
				ansiFile.c_str(),
				(DWORD)ansiFile.size(),
				&written,
				NULL);
		}
	}

	void WriteAt(short x, short y, const char* text, int len, WORD attr=15) {
		SetConsoleCursorPosition(m_hConsole, { x , y });
		SetConsoleTextAttribute(m_hConsole, attr);
		DWORD written;
		WriteConsole(m_hConsole, text, len, &written, NULL);
	}

	void WriteRegisterValueBin(WORD value, const CPUInfo::Coord& coord) {
		static char bits[17];
		bits[16] = 0;

		for (int i = 0; i < coord.w; ++i) {
			bits[16 - i] = (value & (1 << i)) ? '1' : '0';
		}

		WriteAt(coord.x - 1, coord.y, bits + 17 - coord.w, coord.w);
	}

	void WriteRegisterValueHex(WORD value, const CPUInfo::Coord& coord) {
		static char hex[3];
		hex[0] = hexDigits[(value & 0xF0) >> 4];
		hex[1] = hexDigits[(value & 0x0F)];
		hex[2] = 0;

		WriteAt(coord.x - 1, coord.y, hex + 2 - coord.w, coord.w, 14);
	}

	void WriteRegisterValue(WORD value, const char* label) {
		CPUInfo::Coord coord = m_pCPUInfo->GetCoord(label);

		if (coord.IsSet()) {
			WriteRegisterValueBin(value, coord);
		}

		bool twoBits = (coord.w == 2);
		std::string hexLabel = label;
		hexLabel.append(".hex");
		coord = m_pCPUInfo->GetCoord(hexLabel.c_str());
		if (coord.IsSet()) {
			WriteRegisterValueHex(twoBits ?(value&0x03):value, coord);
		}
	}

	void WriteDecimalValue(BYTE value, const char* label) {
		static char decimal[3];
		CPUInfo::Coord coord = m_pCPUInfo->GetCoord(label);

		if (coord.IsSet()) {
			decimal[0] = (value>99) ? ((value / 100) + '0') : ' ';
			decimal[1] = (value>9) ? (((value / 10) % 10) + '0') : ' ';
			decimal[2] = (value % 10) + '0';
			WriteAt(coord.x - 1, coord.y, decimal + 3 - coord.w, coord.w);
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

			WriteAt(coord.x - 1, coord.y + y, ramLine, lineLen);
		}

		// Highlight current xy
		char digit = hexDigits[TMS1000::GetRAM()];
		WriteAt(coord.x - 1 + TMS1000::g_cpu.Y , coord.y + TMS1000::g_cpu.X, &digit, 1, 0xF0);
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

			WriteAt(coord.x - 1, coord.y + y, romLine, lineLen * 2);
		}

		static const CPUInfo::Coord pageCoord = m_pCPUInfo->GetCoord("ROMPage");
		if (pageCoord.IsSet()) {
			WriteRegisterValueHex(TMS1000::g_cpu.PA, pageCoord);
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
			
			WriteAt(coordAddr.x - 1, coordAddr.y + y, line, 3, 14);

			BYTE opcode = TMS1000::g_memory.ROM[PC + baseAddr];
			std::string instr = m_pCPUInfo->Disassemble(opcode);
			memset(line, 0, 16);
			strcpy(line, instr.c_str());

			WriteAt(coordData.x - 1, coordData.y + y, line, coordData.w);
		}
	}

	void WriteStatus() {
		static const CPUInfo::Coord coord = m_pCPUInfo->GetCoord("Status");
		if (!coord.IsSet())
			return;

		static char status[11];

		if (running) {		
			sprintf(status, "%d ticks", TMS1000::GetTicks());
		}
		else {
			sprintf(status, "Step Mode");
		}

		WriteAt(coord.x - 1, coord.y, status, coord.w);
	}

	void UpdateStatus() {
		WriteRegisterValue(TMS1000::g_cpu.X, "X");
		WriteRegisterValue(TMS1000::g_cpu.Y, "Y");
		WriteRegisterValue(TMS1000::g_cpu.A, "A");
		
		WriteDecimalValue(TMS1000::GetM(), "M");

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
		WriteStatus();
	}

	int ReadInput() {
		int key = _getch();
		// Special Char
		if (key == 0 || key == 0xE0)
		{
			key = _getch();
		}
		
		return key;
	}

	void SetRunMode(bool run) {
		running = run;
		WriteStatus();
	}
}