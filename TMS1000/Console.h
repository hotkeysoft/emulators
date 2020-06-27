#pragma once

#include "CPUInfo.h"

namespace Console
{
	void Init();
	void Init(CPUInfo *pCPUInfo);
	void UpdateStatus();
	void SetRunMode(bool run);


	void GotoXY(short x, short y);
	void SetColor(short attr);
	void SetColor(BYTE fore, BYTE back);
	void Write(const char* text);
	void WriteAt(short x, short y, const char* text, int len, WORD attr = 15);

	int ReadInput();
};

