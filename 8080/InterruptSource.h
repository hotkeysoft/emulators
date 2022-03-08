#pragma once

class InterruptSource : virtual public Logger
{
public:
	InterruptSource() {}
	virtual ~InterruptSource() {}

	virtual bool IsInterrupting() = 0;
};

