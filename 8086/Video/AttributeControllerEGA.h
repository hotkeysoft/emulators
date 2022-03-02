#pragma once

#include "../Common.h"
#include "../Serializable.h"
#include <array>

namespace attr_ega
{
	enum class RegisterMode { ADDRESS, DATA };
	enum class PaletteSource { CPU, VIDEO };

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
	};

	class AttrController : public emul::Serializable
	{
	public:
		PaletteSource paletteSource = PaletteSource::CPU;
		RegisterMode currMode = RegisterMode::ADDRESS;
		AttrControllerAddress currRegister = AttrControllerAddress::ATTR_INVALID;

		void ResetMode() { currMode = RegisterMode::ADDRESS; }

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

		// emul::Serializable
		virtual void Serialize(json& to) override
		{
			to["paletteSource"] = paletteSource;
			to["currMode"] = currMode;
			to["currRegister"] = currRegister;
			to["palette"] = palette;
			to["graphics"] = graphics;
			to["monochrome"] = monochrome;
			to["extend8to9"] = extend8to9;
			to["blink"] = blink;
			to["overscanColor"] = overscanColor;
			to["colorPlaneEnable"] = colorPlaneEnable;
			to["videoStatusMux"] = videoStatusMux;
			to["hPelPanning"] = hPelPanning;
		}

		virtual void Deserialize(json& from) override
		{
			paletteSource = from["paletteSource"];
			currMode = from["currMode"];
			currRegister = from["currRegister"];
			palette = from["palette"];
			graphics = from["graphics"];
			monochrome = from["monochrome"];
			extend8to9 = from["extend8to9"];
			blink = from["blink"];
			overscanColor = from["overscanColor"];
			colorPlaneEnable = from["colorPlaneEnable"];
			videoStatusMux = from["videoStatusMux"];
			hPelPanning = from["hPelPanning"];
		}
	};
}
