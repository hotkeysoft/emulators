#pragma once

#include "../CPU/MemoryBlock.h"

using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

namespace cart
{
	class CartridgePCjr : public emul::MemoryBlock
	{
	public:
		CartridgePCjr();

		CartridgePCjr(const CartridgePCjr&) = delete;
		CartridgePCjr& operator=(const CartridgePCjr&) = delete;
		CartridgePCjr(CartridgePCjr&&) = delete;
		CartridgePCjr& operator=(CartridgePCjr&&) = delete;

		virtual bool LoadFromFile(const char* file, WORD offset = 0) override;
		ADDRESS GetBaseAddress() { return emul::S2A(m_header.address); }

	protected:
		const char* const expectedSignature = "PCjr Cartridge image file\r\n";

#pragma pack(push, 1)
		struct header
		{
			char signature[27];
			char creator[32];
			char comment[401];
			BYTE versionH;
			BYTE versionL;
			WORD address;
			WORD adrmask;
			BYTE reserved[46];
		} m_header;
#pragma pack(pop)
	};
}