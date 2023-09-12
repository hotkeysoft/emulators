#pragma once
#include <CPU/MemoryBlockBase.h>
#include "Device8530.h"

namespace scc::mac
{
	// SCC is mapped to addresses $800000-$AFFFFF
	// But $800000-$8FFFFF / $A00000-$AFFFFF conflicts 
	// with ROM on bus

	class IOBlockSCCMac : public emul::MemoryBlockBase
	{
	public:
		IOBlockSCCMac() : MemoryBlockBase("IO_SCC", 0x400000, emul::MemoryType::IO) {}
		virtual ~IOBlockSCCMac() {};

		void Init(Device8530* scc) { m_scc = scc; }

	protected:

		// SCC Read: A21 = 0
		virtual BYTE read(emul::ADDRESS offset) const override
		{
			assert(m_scc);
			if (emul::GetBit(offset, 21) == false)
			{
				if (emul::GetLSB(offset))
				{
					m_scc->Reset();
				}
				else
				{
					SCCChannel& ch = emul::GetBit(offset, 1) ? m_scc->GetChannelA() : m_scc->GetChannelB();
					return emul::GetBit(offset, 2) ? ch.ReadData() : ch.ReadControl();
				}
			}

			return 0xFF;
		}

		// SCC Read: A21 = 1
		virtual void write(emul::ADDRESS offset, BYTE data) override
		{
			assert(m_scc);
			if (emul::GetBit(offset, 21))
			{
				SCCChannel& ch = emul::GetBit(offset, 1) ? m_scc->GetChannelA() : m_scc->GetChannelB();
				emul::GetBit(offset, 2) ? ch.WriteData(data) : ch.WriteControl(data);
			}
		}

		scc::Device8530* m_scc = nullptr;
    };
}