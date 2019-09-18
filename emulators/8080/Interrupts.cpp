#include "StdAfx.h"
#include "Interrupts.h"

Interrupts::Interrupts() : Logger("INTR")
{
	for (int i = 0; i<MAXINTERRUPT; i++)
	{
		m_interrupts[i] = NULL;
	}
}

Interrupts::~Interrupts()
{
}

bool Interrupts::Allocate(BYTE intNb, InterruptSource * intSource)
{
	LogPrintf(LOG_INFO, "Request to allocate interrupt source #%d", intNb);

	if (m_interrupts[intNb] != NULL)
	{
		LogPrintf(LOG_ERROR, "Interrupt already exists");
		return false;
	}

	for (int i = 0; i<MAXINTERRUPT; i++)
	{
		if (m_interrupts[i] == intSource)
		{
			LogPrintf(LOG_INFO, "Object already allocated at #%d", i);
			return false;
		}
	}

	m_interrupts[intNb] = intSource;

	return true;
}

bool Interrupts::Free(InterruptSource * intSource)
{
	for (int i = 0; i<MAXINTERRUPT; i++)
	{
		if (m_interrupts[i] == intSource)
		{
			LogPrintf(LOG_INFO, "Freeing interrupt #%d", i);
			m_interrupts[i] = NULL;
			return true;
		}
	}

	LogPrintf(LOG_ERROR, "Interrupts::Free: interrupt source not found");
	return false;
}

bool Interrupts::IsInterrupting(BYTE intNb)
{
	if (m_interrupts[intNb] == NULL)
	{
		return false;
	}

	return m_interrupts[intNb]->IsInterrupting();
}
