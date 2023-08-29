#pragma once
#include <CPU/MemoryBlockBase.h>
#include "DeviceIWM.h"

namespace floppy::woz
{
	// IWM is mapped to addresses $C00000-$DFFFFF
	// But $C00000-$CFFFFF collide with ROM

	// Select lines are A12-A9
	// A0 = 1 (IWM uses lower byte of the data bus)
	class IOBlockIWMMac : public emul::MemoryBlockBase
	{
	public:
		IOBlockIWMMac() : MemoryBlockBase("IO_IWM", 0x100000, emul::MemoryType::IO) {}
		virtual ~IOBlockIWMMac() {};

		void Init(DeviceIWM* iwm) { m_iwm = iwm; }

	protected:
		virtual BYTE read(emul::ADDRESS offset) const override
		{
			assert(m_iwm);
			if (emul::GetLSB(offset))
			{
				m_iwm->SetStateRegister((offset >> 9) & 15);
				return m_iwm->Read();
			}
			else
			{
				return 0xFF;
			}
		}

		virtual void write(emul::ADDRESS offset, BYTE data) override
		{
			assert(m_iwm);
			if (emul::GetLSB(offset))
			{
				m_iwm->SetStateRegister((offset >> 9) & 15);
				m_iwm->Write(data);
			}
		}

		DeviceIWM* m_iwm = nullptr;
    };
}