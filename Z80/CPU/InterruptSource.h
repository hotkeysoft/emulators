#pragma once

class InterruptSource : virtual public Logger
{
public:
	InterruptSource() {}
	virtual ~InterruptSource() {}

	virtual const char* GetID() = 0;
	virtual bool IsInterrupting() = 0;
	virtual emul::ADDRESS GetVector() = 0;
	virtual void Clear() {};	
};

