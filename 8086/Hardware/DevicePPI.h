#pragma once

#include "../CPU/PortConnector.h"

// Base class for various IO peripherals devices (8255, 8042)

namespace ppi
{
	class DevicePPI : public emul::PortConnector
	{
	public:
		virtual ~DevicePPI() {}

		DevicePPI() = delete;
		DevicePPI(const DevicePPI&) = delete;
		DevicePPI& operator=(const DevicePPI&) = delete;
		DevicePPI(DevicePPI&&) = delete;
		DevicePPI& operator=(DevicePPI&&) = delete;

		virtual void Init() {}
		virtual void Reset() {}

		virtual bool IsSoundON() = 0;

		virtual void SetCurrentKeyCode(BYTE keyCode) = 0;

	protected:
		DevicePPI(WORD baseAddress) : m_baseAddress(baseAddress) {}

		const WORD m_baseAddress = 0;
	};
}
