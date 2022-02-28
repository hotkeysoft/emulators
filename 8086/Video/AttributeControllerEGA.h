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

		std::array<uint32_t, 16> palette = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		uint32_t overscanColor = 0;

		// emul::Serializable
		virtual void Serialize(json& to) override
		{
			to["paletteSource"] = paletteSource;
			to["currMode"] = currMode;
			to["currRegister"] = currRegister;
			to["palette"] = palette;
			to["overscanColor"] = overscanColor;
		}

		virtual void Deserialize(json& from) override
		{
			paletteSource = from["paletteSource"];
			currMode = from["currMode"];
			currRegister = from["currRegister"];
			palette = from["palette"];
			overscanColor = from["overscanColor"];
		}
	};
}
