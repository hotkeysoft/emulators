#include "Console.h"
#include <stdio.h>
#include <fstream>
#include <conio.h>

Console::Console()
{
	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

Console::~Console()
{
}

void Console::Init(short columns) 
{
	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_FONT_INFOEX cfi = { sizeof(cfi) };
	// Populate cfi with the screen buffer's current font info
	GetCurrentConsoleFontEx(m_hConsole, FALSE, &cfi);

	// Modify the font size in cfi
	cfi.dwFontSize.X = 18;
	cfi.dwFontSize.Y = 18;

	// Use cfi to set the screen buffer's new font
	SetCurrentConsoleFontEx(m_hConsole, FALSE, &cfi);

	SMALL_RECT windowSize = { 0, 0, columns-1, 25 };
	if (!SetConsoleWindowInfo(m_hConsole, TRUE, &windowSize)) {
		fprintf(stderr, "SetConsoleWindowInfo failed %d\n", GetLastError());
	}
	if (!SetConsoleScreenBufferSize(m_hConsole, { columns, 26 })) {
		fprintf(stderr, "SetConsoleScreenBufferSize failed %d\n", GetLastError());
	}

	CONSOLE_CURSOR_INFO cursorInfo = { 1, FALSE };
	SetConsoleCursorInfo(m_hConsole, &cursorInfo);

	SetConsoleOutputCP(437);

	DWORD dwMode = 0;
	GetConsoleMode(m_hConsole, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(m_hConsole, dwMode);
}

void Console::Clear()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD count;
	DWORD cellCount;
	COORD homeCoords = { 0, 0 };

	if (!GetConsoleScreenBufferInfo(m_hConsole, &csbi)) return;
	cellCount = csbi.dwSize.X * csbi.dwSize.Y;

	FillConsoleOutputCharacterA(m_hConsole, ' ', cellCount, homeCoords, &count);
	FillConsoleOutputAttribute(m_hConsole, csbi.wAttributes, cellCount, homeCoords, &count);

	SetConsoleCursorPosition(m_hConsole, homeCoords);
}

void Console::WriteAt(short x, short y, const char* text, size_t len, WORD attr)
{
	SetConsoleCursorPosition(m_hConsole, { x - 1 , y - 1 });
	SetConsoleTextAttribute(m_hConsole, attr);
	DWORD written;
	if (len == SIZE_MAX)
	{
		len = strlen(text);
	}
	WriteConsoleA(m_hConsole, text, len, &written, NULL);
}

void Console::WriteAt(short x, short y, char ch, WORD attr)
{
	COORD pos{ x, y };
	DWORD dummy;
	const char data = ch;
	WriteConsoleOutputCharacterA(m_hConsole, &ch, 1, pos, &dummy);
	WriteConsoleOutputAttribute(m_hConsole, &attr, 1, pos, &dummy);
}

void Console::WriteBuffer(const char* buf, size_t bufSize)
{
	SetConsoleCursorPosition(m_hConsole, { 0, 0 });
	DWORD written;
	WriteConsoleA(m_hConsole,
		buf,
		(DWORD)bufSize,
		&written,
		NULL);
}