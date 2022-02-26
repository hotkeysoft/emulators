#pragma once
#include "../CPU/MemoryBlock.h"

namespace memory_ega
{
	enum class RAMSIZE { EGA_64K = 65536, EGA_128K = 131072, EGA_256K = 262144 };

	class MemoryEGA : public emul::MemoryBlock
	{
	public:
		MemoryEGA(RAMSIZE ramsize);
		virtual ~MemoryEGA() {}

		void SetWriteMode(BYTE mode) { m_writeMode = std::min(mode, (BYTE)2); }
		void SetReadMode(bool readModeCompare) { m_readModeCompare = readModeCompare; }
		void SetReadPlane(BYTE plane) { m_currReadPlane = plane & 3; }

		virtual BYTE read(emul::ADDRESS offset) override;
		virtual emul::BytePtr getPtr(emul::ADDRESS offset) override;
		virtual void write(emul::ADDRESS offset, BYTE data) override;

	protected:
		const DWORD m_planeSize;
		const emul::ADDRESS m_planeAddressMask;
		
		MemoryBlock m_planes[4];
		BYTE m_currReadPlane = 0;

		bool m_readModeCompare = false;
		BYTE m_writeMode = 0;
	};
}
