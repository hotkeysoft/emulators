#include "stdafx.h"

#include "Device8255CPC464.h"

namespace ppi
{
	BYTE Device8255CPC464::PORTA_IN()
	{
		LogPrintf(LOG_INFO, "PORTA: IN");
		return 0xFF;
	}
	void Device8255CPC464::PORTA_OUT(BYTE value)
	{
		LogPrintf(LOG_INFO, "PORTA: OUT, value=%02x", value);
	}

	BYTE Device8255CPC464::PORTB_IN()
	{
		LogPrintf(LOG_INFO, "PORTB: IN");
		return 0xFF;
	}
	void Device8255CPC464::PORTB_OUT(BYTE value)
	{
		LogPrintf(LOG_INFO, "PORTB: OUT, value=%02x", value);
	}

	BYTE Device8255CPC464::PORTC_IN()
	{
		LogPrintf(LOG_INFO, "PORTC: IN");
		return 0xFF;
	}
	void Device8255CPC464::PORTC_OUT(BYTE value)
	{
		LogPrintf(LOG_INFO, "PORTC: OUT, value=%02x", value);
	}

}
