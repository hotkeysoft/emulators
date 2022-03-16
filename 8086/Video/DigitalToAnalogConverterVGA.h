#pragma once

#include "../Common.h"
#include "../Serializable.h"
#include "../CPU/PortConnector.h"

#include <array>

namespace dac_vga
{
	struct DACData
	{
		BYTE pelMask = 0xFF;

		// Palette registers
		std::array<uint32_t, 256> palette;
	};

	class DigitalToAnalogConverter : public emul::PortConnector, public emul::Serializable
	{
	public:
		DigitalToAnalogConverter(WORD baseAddress);

		DigitalToAnalogConverter() = delete;
		DigitalToAnalogConverter(const DigitalToAnalogConverter&) = delete;
		DigitalToAnalogConverter& operator=(const DigitalToAnalogConverter&) = delete;
		DigitalToAnalogConverter(DigitalToAnalogConverter&&) = delete;
		DigitalToAnalogConverter& operator=(DigitalToAnalogConverter&&) = delete;

		virtual void Init();
		virtual void Reset();

		void ConnectPorts();
		void DisconnectPorts();

		uint32_t GetColor(BYTE index) const { return m_data.palette[index]; }

		const DACData& GetData() const { return m_data; }

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		const WORD m_baseAddress;

		enum class Mode { WRITE = 0, READ = 3 } m_currMode = Mode::READ;

		WORD m_readAddress = 0;
		WORD m_writeAddress = 0;

		// Pel mask
		BYTE ReadPelMask();
		void WritePelMask(BYTE value);

		// DAC State
		BYTE ReadDACState();

		// Read Mode: Write Palette Address
		void WriteAddressR(BYTE value);

		// Write Mode: Read/Write Palette Address
		BYTE ReadAddressW();
		void WriteAddressW(BYTE value);

		// Palette Data
		BYTE ReadPaletteData();
		void WritePaletteData(BYTE value);

		DACData m_data;
	};
}
