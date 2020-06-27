#pragma once

#include "Logger.h"

class InterruptSource : virtual public Logger
{
public:
	InterruptSource() {}
	virtual ~InterruptSource() {}

	virtual bool IsInterrupting() = 0;
};

