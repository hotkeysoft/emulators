#pragma once

#include <CPU/PortConnector.h>
#include <Serializable.h>

namespace dma
{
	class Device8237;

	enum class PageRegister
	{
		DMA0 = 0x7,
		DMA1 = 0x3,
		DMA2 = 0x1,
		DMA3 = 0x2,
		DMA5 = 0xB,
		DMA6 = 0x9,
		DMA7 = 0xA,
		REFRESH = 0xF
	};

	class DeviceDMAPageRegister : public emul::PortConnector
	{
	public:
		DeviceDMAPageRegister(WORD baseAddress);

		DeviceDMAPageRegister() = delete;
		DeviceDMAPageRegister(const DeviceDMAPageRegister&) = delete;
		DeviceDMAPageRegister& operator=(const DeviceDMAPageRegister&) = delete;
		DeviceDMAPageRegister(DeviceDMAPageRegister&&) = delete;
		DeviceDMAPageRegister& operator=(DeviceDMAPageRegister&&) = delete;

		void Init(Device8237* dmaPrimary, Device8237* dmaSecondary = nullptr);

	protected:
		WORD m_baseAddress;
		BYTE m_pageMask = 0x0F; // Default for 20 bit address bus

		std::array<BYTE, 16> m_registers = {};

		BYTE ReadRegister();
		void WriteRegister(BYTE value);

		Device8237* m_dmaPrimary = nullptr;
		Device8237* m_dmaSecondary = nullptr;
	};
}


