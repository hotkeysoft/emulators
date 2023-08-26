#pragma once
#include <CPU/MemoryBlockBase.h>
#include "Device6522Mac.h"

namespace via
{
	// VIA is mapped to addresses $E00000-$EFFFFF
	// But E00000-$E7FFFF collide with Phase Read

	// Select lines are A12-A9
	// A0 = 0 (VIA uses upper byte of the data bus)
	class IOBlockVIAMac : public emul::MemoryBlockBase
	{
	public:
		IOBlockVIAMac() : MemoryBlockBase("IO_VIA", 0x80000, emul::MemoryType::IO) {}
		virtual ~IOBlockVIAMac() {};

		void Init(Device6522Mac* via) { m_via = via; }

	protected:
		virtual BYTE read(emul::ADDRESS offset) const override
		{
			assert(m_via);
			return emul::GetLSB(offset) ? 0xFF : m_via->ReadRegister((offset >> 9) & 15);
		}

		virtual void write(emul::ADDRESS offset, BYTE data) override
		{
			assert(m_via);
			if (!emul::GetLSB(offset))
			{
				m_via->WriteRegister((offset >> 9) & 15, data);
			}
		}

		Device6522Mac* m_via = nullptr;
    };
}