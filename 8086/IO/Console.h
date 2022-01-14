#pragma once
#include "../Common.h"

#include "windows.h"
#include "wincon.h"

class Console
{
public:
	Console();
	~Console();

	void Init(short columns = 80, short fontSize = 18);

	void Clear();

	void WaitForKey();

	void WriteAt(short x, short y, const char* text, size_t len = SIZE_MAX, WORD attr = 15);
	void WriteAt(short x, short y, char ch, WORD attr);
	void WriteBuffer(const char* buf, size_t bufSize);

	void WriteAttrAt(short x, short y, const WORD* attr, size_t len);
	void WriteAttrAt(short x, short y, const WORD attr, size_t len);

private:
	HANDLE m_hConsole;
};
