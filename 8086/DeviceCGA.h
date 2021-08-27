#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Logger.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace cga
{
	class DeviceCGA : public PortConnector
	{
	public:
		DeviceCGA(WORD baseAddress);

		DeviceCGA() = delete;
		DeviceCGA(const DeviceCGA&) = delete;
		DeviceCGA& operator=(const DeviceCGA&) = delete;
		DeviceCGA(DeviceCGA&&) = delete;
		DeviceCGA& operator=(DeviceCGA&&) = delete;

		void Init();
		void Reset();

		void Tick();

		BYTE IN();
		void OUT(BYTE value);

		BYTE ReadStatus();

	protected:
		const WORD m_baseAddress;

		bool IsHSync() { return m_hPos > 320; }
		bool IsVSync() { return m_vPos > 200; }

		WORD m_hPos;
		WORD m_vPos;
	};
}
