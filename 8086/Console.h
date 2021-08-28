#pragma once
#include "Common.h"

#include "windows.h"
#include "wincon.h"

class Console
{
public:
	Console();
	~Console();

	void Init(short columns = 80);

	void WriteAt(short x, short y, const char* text, int len, WORD attr = 15);
	void WriteAt(short x, short y, char ch, WORD attr);
	void WriteBuffer(const char* buf, size_t bufSize);

private:	
	HANDLE m_hConsole;
};
