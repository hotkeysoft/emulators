#pragma once

#include "CPUInfo.h"

namespace Console
{
	void Init(CPUInfo *pCPUInfo);
	void UpdateStatus();
	void SetRunMode(bool run);

	int ReadInput();
};

