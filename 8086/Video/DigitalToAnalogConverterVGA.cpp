#include "stdafx.h"

#include "DigitalToAnalogConverterVGA.h"

using emul::GetBit;

namespace dac_vga
{
	// Pos:
	// 0: A
	// 1: R
	// 2: G
	// 3: B
	void SetARGB(uint32_t& dest, int pos, BYTE val)
	{
		assert(pos < 4);
		BYTE* begin = reinterpret_cast<BYTE*>(&dest);
		begin[3 - pos] = (val << 2);
	}

	BYTE GetARGB(const uint32_t src, int pos)
	{
		assert(pos < 4);
		const BYTE* begin = reinterpret_cast<const BYTE*>(&src);
		return begin[3 - pos] >> 2;
	}

	DigitalToAnalogConverter::DigitalToAnalogConverter(WORD baseAddress) :
		Logger("dacVGA"),
		m_baseAddress(baseAddress)
	{
	}

	void DigitalToAnalogConverter::Reset()
	{
	}

	void DigitalToAnalogConverter::Init()
	{
		Reset();
	}

	void DigitalToAnalogConverter::ConnectPorts()
	{
		// Pel Mask
		Connect(m_baseAddress + 0x6, static_cast<PortConnector::INFunction>(&DigitalToAnalogConverter::ReadPelMask));
		Connect(m_baseAddress + 0x6, static_cast<PortConnector::OUTFunction>(&DigitalToAnalogConverter::WritePelMask));

		// DAC State
		Connect(m_baseAddress + 0x7, static_cast<PortConnector::INFunction>(&DigitalToAnalogConverter::ReadDACState));

		// Read Mode: Write Palette Address
		Connect(m_baseAddress + 0x7, static_cast<PortConnector::OUTFunction>(&DigitalToAnalogConverter::WriteAddressR));

		// Write Mode: Read/Write Palette Address
		Connect(m_baseAddress + 0x8, static_cast<PortConnector::INFunction>(&DigitalToAnalogConverter::ReadAddressW));
		Connect(m_baseAddress + 0x8, static_cast<PortConnector::OUTFunction>(&DigitalToAnalogConverter::WriteAddressW));

		// Palette Data
		Connect(m_baseAddress + 0x9, static_cast<PortConnector::INFunction>(&DigitalToAnalogConverter::ReadPaletteData));
		Connect(m_baseAddress + 0x9, static_cast<PortConnector::OUTFunction>(&DigitalToAnalogConverter::WritePaletteData));
	}

	void DigitalToAnalogConverter::DisconnectPorts()
	{
		DisconnectInput(m_baseAddress + 0x6);
		DisconnectOutput(m_baseAddress + 0x6);
		DisconnectInput(m_baseAddress + 0x7);
		DisconnectOutput(m_baseAddress + 0x7);
		DisconnectInput(m_baseAddress + 0x8);
		DisconnectOutput(m_baseAddress + 0x8);
		DisconnectInput(m_baseAddress + 0x9);
		DisconnectOutput(m_baseAddress + 0x9);
	}

	// Read-only in the IBM VGA spec
	BYTE DigitalToAnalogConverter::ReadPelMask()
	{
		return m_data.pelMask;
	}

	void DigitalToAnalogConverter::WritePelMask(BYTE value)
	{
		LogPrintf(LOG_INFO, "WritePelMask, value=%02x", value);
		m_data.pelMask = value;
	}

	BYTE DigitalToAnalogConverter::ReadDACState()
	{
		return (BYTE)m_currMode;
	}

	void DigitalToAnalogConverter::WriteAddressR(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteAddress (Read Mode), value=%02x", value);

		m_readAddress = value << 2;
		m_currMode = Mode::READ;
	}

	BYTE DigitalToAnalogConverter::ReadAddressW()
	{
		return m_writeAddress >> 2;
	}

	void DigitalToAnalogConverter::WriteAddressW(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteAddress (Write Mode), value=%02x", value);

		m_writeAddress = value << 2;
		m_currMode = Mode::WRITE;
	}

	// TODO: Value is 6 bits, shift to 8
	BYTE DigitalToAnalogConverter::ReadPaletteData()
	{
		BYTE index = m_readAddress >> 2;
		BYTE rgb = m_readAddress & 3;
		LogPrintf(LOG_INFO, "ReadPaletteData, palette[%d][%d]", index, rgb);

		BYTE value = GetARGB(m_data.palette[index], rgb + 1);

		++m_readAddress;
		if ((m_readAddress & 3) == 3)
		{
			// Skip alpha
			++m_readAddress;
		}

		return value;
	}

	// TODO: Value is 6 bits, shift to 8
	void DigitalToAnalogConverter::WritePaletteData(BYTE value)
	{
		BYTE index = m_writeAddress >> 2;
		BYTE rgb = m_writeAddress & 3;
		LogPrintf(LOG_INFO, "WritePaletteData, palette[%d][%d] = %d", index, rgb, value);

		SetARGB(m_data.palette[index], rgb + 1, value);

		++m_writeAddress;
		if ((m_writeAddress & 3) == 3)
		{
			// Set alpha byte
			SetARGB(m_data.palette[index], 0, 0xFF);
			LogPrintf(LOG_DEBUG, "WritePaletteData, palette[%d] = %08x", index, m_data.palette[index]);
			// Move to next
			++m_writeAddress;
		}
	}

	void DigitalToAnalogConverter::Serialize(json& to)
	{
		to["currMode"] = m_currMode;
		to["writeAddress"] = m_writeAddress;
		to["readAddress"] = m_readAddress;

		to["pelMask"] = m_data.pelMask;
		to["palette"] = m_data.palette;
	}

	void DigitalToAnalogConverter::Deserialize(const json& from)
	{
		m_currMode = from["currMode"];
		m_writeAddress = from["writeAddress"];
		m_readAddress = from["readAddress"];

		m_data.pelMask = from["pelMask"];
		m_data.palette = from["palette"];
	}
}
