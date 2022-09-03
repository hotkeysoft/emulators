#pragma once

#include "InterruptSource.h"
#include <list>

namespace emul
{
	const int MAXINTERRUPT = 16;

	class Interrupts : public Logger
	{
	public:
		Interrupts();
		virtual ~Interrupts();

		bool Allocate(BYTE intNb, InterruptSource* intSource);
		bool Free(InterruptSource* intSource);

		bool IsInterrupting(BYTE intNb);

	private:
		InterruptSource* m_interrupts[MAXINTERRUPT];
	};
}