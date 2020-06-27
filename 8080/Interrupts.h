#pragma once

#include "Common.h"
#include "InterruptSource.h"
#include "Logger.h"
#include <list>

const int MAXINTERRUPT = 16;

class Interrupts : public Logger
{
public:
	Interrupts();
	virtual ~Interrupts();

	bool Allocate(BYTE intNb, InterruptSource *intSource);
	bool Free(InterruptSource *intSource);

	bool IsInterrupting(BYTE intNb);

private:
	InterruptSource* m_interrupts[MAXINTERRUPT];
};
