#pragma once

#include "../Common.h"
#include "../Serializable.h"
#include "../CPU/PortConnector.h"

#include <array>

namespace attr_ega
{
	enum class PaletteSource { CPU, VIDEO };
	enum class ColorMode { RGB4, RGB6 };

	struct AttrControllerData
	{
		PaletteSource paletteSource = PaletteSource::CPU;

		// Palette registers
		std::array<uint32_t, 16> palette = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		// Mode control register
		bool graphics = false; // 0: alpha, 1: graphics
		bool monochrome = false; // 1: Use mono display attributes
		bool extend8to9 = false; // For alpha 9px char width, extend 8th pixel for chars 0xC0-0xDF (mono/MDA emulation)
		bool blink = false; // 0: ATR3 = hi bg color, 1: blink

		// Overscan control register
		uint32_t overscanColor = 0;

		// Color plane enable register
		BYTE colorPlaneEnable = 0x0F;
		BYTE videoStatusMux = 0;

		// Horizontal Pixel Panning register
		BYTE hPelPanning = 0;
	};

	class AttrController : public emul::PortConnector, public emul::Serializable
	{
	public:
		AttrController(WORD baseAddress);

		AttrController() = delete;
		AttrController(const AttrController&) = delete;
		AttrController& operator=(const AttrController&) = delete;
		AttrController(AttrController&&) = delete;
		AttrController& operator=(AttrController&&) = delete;

		virtual void Init();
		virtual void Reset();

		const AttrControllerData& GetData() const { return m_data; }

		void ResetMode() { m_currMode = RegisterMode::ADDRESS; }
		void SetColorMode(ColorMode mode) { m_colorMode = mode; }

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		const WORD m_baseAddress;

		uint32_t GetRGB32Color(BYTE value);

		enum class RegisterMode
		{ 
			ADDRESS, 
			DATA 
		} m_currMode = RegisterMode::ADDRESS;

		enum class AttrControllerAddress
		{
			ATTR_PALETTE_MIN = 0x00,
			ATTR_PALETTE_MAX = 0x0F,

			ATTR_MODE_CONTROL = 0x10,
			ATTR_OVERSCAN_COLOR = 0x11,
			ATTR_COLOR_PLANE_EN = 0x12,
			ATTR_H_PEL_PANNING = 0x13,

			_ATTR_MAX = ATTR_H_PEL_PANNING,
			ATTR_INVALID = 0xFF
		} m_currRegister = AttrControllerAddress::ATTR_INVALID;

		void WriteData(BYTE value);

		ColorMode m_colorMode = ColorMode::RGB4;
		
		AttrControllerData m_data;
	};
}
