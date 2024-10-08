#include "stdafx.h"

#include "Console.h"

Console::Console()
{
	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

Console::~Console()
{
}

void Console::Init(short columns, short fontSize)
{
	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_FONT_INFOEX cfi = { sizeof(cfi) };
	// Populate cfi with the screen buffer's current font info
	GetCurrentConsoleFontEx(m_hConsole, FALSE, &cfi);

	// Modify the font size in cfi
	cfi.dwFontSize.X = fontSize;
	cfi.dwFontSize.Y = fontSize;

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

void Console::Clear(short x, short y, short w, short h, char ch, WORD attr)
{
	DWORD count;
	DWORD cellCount;
	COORD homeCoords = { x - 1, y - 1 };

	cellCount = w;

	for (auto line = 0; line < h; ++line, homeCoords.Y++)
	{
		FillConsoleOutputCharacterA(m_hConsole, ch, cellCount, homeCoords, &count);
		FillConsoleOutputAttribute(m_hConsole, attr, cellCount, homeCoords, &count);
	}
}

void Console::WaitForKey()
{
	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	while (1)
	{
		switch (WaitForSingleObject(hStdIn, INFINITE))
		{
		case WAIT_OBJECT_0:
			if (_kbhit())
			{
				return;
			}
			else // Some other event, clear it from the queue
			{
				INPUT_RECORD r[512];
				DWORD read;
				ReadConsoleInput(hStdIn, r, 512, &read);
			}
		}
	}
}

void Console::WriteAt(short x, short y, const char* text, size_t len, WORD attr)
{
	COORD pos = { x - 1, y - 1 };
	SetConsoleCursorPosition(m_hConsole, pos);
	SetConsoleTextAttribute(m_hConsole, attr);
	DWORD written;
	if (len == SIZE_MAX)
	{
		len = strlen(text);
	}
	WriteConsoleA(m_hConsole, text, (DWORD)len, &written, NULL);
}

void Console::WriteAttrAt(short x, short y, const WORD* attr, size_t len)
{
	COORD pos = { x - 1, y - 1 };
	DWORD dummy;

	WriteConsoleOutputAttribute(m_hConsole, attr, (DWORD)len, pos, &dummy);
}

void Console::WriteAttrAt(short x, short y, const WORD attr, size_t len)
{
	COORD pos = { x - 1, y - 1 };
	DWORD dummy;

	FillConsoleOutputAttribute(m_hConsole, attr, (DWORD)len, pos, &dummy);
}

void Console::WriteAt(short x, short y, char ch, WORD attr)
{
	COORD pos = { x - 1, y - 1 };
	DWORD dummy;
	const char data = ch;
	WriteConsoleOutputCharacterA(m_hConsole, &ch, 1, pos, &dummy);
	WriteConsoleOutputAttribute(m_hConsole, &attr, 1, pos, &dummy);
}

void Console::WriteBuffer(const char* buf, size_t bufSize)
{
	SetConsoleCursorPosition(m_hConsole, { 0, 0 });
	DWORD written;
	WriteConsoleA(m_hConsole, buf, (DWORD)bufSize, &written, nullptr);
}

void Console::MoveBlockY(short x, short y, short w, short h, short newY)
{
	std::vector<CHAR_INFO> buffer((size_t)w * h);

	SMALL_RECT src;
	src.Top = y;
	src.Left = x - 1;
	src.Bottom = y + h - 1;
	src.Right = x + w + 1;

	COORD bufferSize{ w, h };
	COORD bufferCoord{ 0, 0 };

	ReadConsoleOutput(m_hConsole,
		&buffer[0],
		bufferSize,
		bufferCoord,
		&src);

	SMALL_RECT dest = src;
	dest.Top = newY;
	dest.Bottom = newY + h;

	WriteConsoleOutput(m_hConsole,
		&buffer[0],
		bufferSize,
		bufferCoord,
		&dest);
}
