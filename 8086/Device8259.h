#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Logger.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace pic
{
	class Device8259 : public PortConnector
	{
	public:
		Device8259(WORD baseAddress);

		Device8259() = delete;
		Device8259(const Device8259&) = delete;
		Device8259& operator=(const Device8259&) = delete;
		Device8259(Device8259&&) = delete;
		Device8259& operator=(Device8259&&) = delete;

		void Init();
		void Reset();

		void Tick();

		BYTE IN();
		void OUT(BYTE value);

		BYTE Mask_IN();
		void Mask_OUT(BYTE value);

	protected:
		const WORD m_baseAddress;

		BYTE m_mask;
	};
}
