#include "stdafx.h"
#include "DeviceDMAPageRegister.h"
#include "Device8237.h"

namespace dma
{
	DeviceDMAPageRegister::DeviceDMAPageRegister(WORD baseAddress) :
		Logger("dmaPage"),
		m_baseAddress(baseAddress)
	{
	}

	void DeviceDMAPageRegister::Init(Device8237* dmaPrimary, Device8237* dmaSecondary)
	{
		// Page mask for 20 bit address bus:
		// 
		// Page Register               | DMA Controller (16 bit address)
		// 0 0 0 0 A20 A19 A18 A17 A16 | A15 A14 A13 A12 A11 A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0

		// Page mask for 24 bit address bus, bit 0 is cleared
		// because A16 is part of the shifted DMA controller address:
		// 
		// Page Register                      | DMA Controller (16 bit address)
		// A23 A22 A21 A20 A19 A18 A17 A16(0) | A16 A14 A13 A12 A11 A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 | A0 (always 0)

		assert(dmaPrimary);
		m_dmaPrimary = dmaPrimary;
		m_dmaSecondary = dmaSecondary;

		// See note above, depending of the number of dma controllers, 
		// we assume the machine type:
		// 1 controller:  XT-class machine with a 20 bit data bus, 
		// 2 controllers: AT-class machine with a 24 bit data bus

		m_pageMask = m_dmaSecondary ? 0xFE : 0x0F;

		for (int i = 0; i < 16; ++i)
		{
			Connect(m_baseAddress + i, static_cast<PortConnector::INFunction>(&DeviceDMAPageRegister::ReadRegister));
			Connect(m_baseAddress + i, static_cast<PortConnector::OUTFunction>(&DeviceDMAPageRegister::WriteRegister));
		}
	}

	BYTE DeviceDMAPageRegister::ReadRegister()
	{
		WORD address = GetCurrentPort() - m_baseAddress;
		BYTE value = m_registers[address];
		LogPrintf(LOG_INFO, "ReadRegister, address[%02x] = %02x", address, value);
		return value;
	}

	void DeviceDMAPageRegister::WriteRegister(BYTE value)
	{
		WORD address = GetCurrentPort() - m_baseAddress;
		m_registers[address] = value;

		LogPrintf(LOG_INFO, "WriteRegister, address[%02x] = %02x", address, value);

		value &= m_pageMask;
		switch ((PageRegister)address)
		{
		case PageRegister::DMA0:
			LogPrintf(LOG_INFO, "Set DMA Channel 0 Page [%02x]", value);
			m_dmaPrimary->GetChannel(0).SetPage(value);
			break;

		case PageRegister::DMA1:
			LogPrintf(LOG_INFO, "Set DMA Channel 1 Page [%02x]", value);
			m_dmaPrimary->GetChannel(1).SetPage(value);
			break;

		case PageRegister::DMA2:
			LogPrintf(LOG_INFO, "Set DMA Channel 2 Page [%02x]", value);
			m_dmaPrimary->GetChannel(2).SetPage(value);
			break;

		case PageRegister::DMA3:
			LogPrintf(LOG_INFO, "Set DMA Channel 3 Page [%02x]", value);
			m_dmaPrimary->GetChannel(3).SetPage(value);
			break;

		case PageRegister::DMA5:
			LogPrintf(LOG_INFO, "Set DMA Channel 5 Page [%02x]", value);
			m_dmaSecondary->GetChannel(1).SetPage(value);
			break;
		case PageRegister::DMA6:
			LogPrintf(LOG_INFO, "Set DMA Channel 6 Page [%02x]", value);
			m_dmaSecondary->GetChannel(2).SetPage(value);
			break;
		case PageRegister::DMA7:
			LogPrintf(LOG_INFO, "Set DMA Channel 7 Page [%02x]", value);
			m_dmaSecondary->GetChannel(3).SetPage(value);
			break;

		case PageRegister::REFRESH:
			// TODO: What do we do with this?
			LogPrintf(LOG_INFO, "Set Refresh Page [%02x]", value);
			break;
		default:
			// Nothing to do
			break;
		}
	}
}
