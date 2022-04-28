#include "stdafx.h"
#include "Interrupts8080.h"

namespace emul
{
	bool Interrupts8080::IsInterrupting() const
	{
		return false;
	}

	void Interrupts8080::Acknowledge()
	{
	}
}