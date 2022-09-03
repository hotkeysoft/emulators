#pragma once

#include <Common.h>
#include "../CPU/PortConnector.h"

using emul::PortConnector;
using emul::BYTE;
using emul::WORD;

namespace rtc
{
	class Device8167 : public PortConnector
	{
	public:
		Device8167(WORD baseAddress);
		virtual ~Device8167() {}

		Device8167() = delete;
		Device8167(const Device8167&) = delete;
		Device8167& operator=(const Device8167&) = delete;
		Device8167(Device8167&&) = delete;
		Device8167& operator=(Device8167&&) = delete;

		virtual void Init();

	protected:
		const WORD m_baseAddress;

		BYTE ReadCounterMilliSeconds(); // Not used, in theory
		BYTE ReadCounterDeciCentiSeconds();
		BYTE ReadCounterSeconds();
		BYTE ReadCounterMinutes();
		BYTE ReadCounterHours();
		BYTE ReadCounterDayOfWeek();
		BYTE ReadCounterDayOfMonth();
		BYTE ReadCounterMonth();

		BYTE ReadRAM0(); // Used as month copy - varies from driver to driver
		BYTE ReadRAM1(); // Year, format varies

		BYTE DummyRead();
		void DummyWrite(BYTE value);

		void UpdateCurrentTime();

		struct tm m_now;

	};
}
