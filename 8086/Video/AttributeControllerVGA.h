#pragma once

#include <CPU/CPUCommon.h>
#include <Serializable.h>
#include "../CPU/PortConnector.h"

#include <array>

namespace attr_vga
{
	enum class PaletteSource { CPU, VIDEO };

	struct AttrControllerData
	{
		PaletteSource paletteSource = PaletteSource::CPU;

		// Palette registers
		std::array<BYTE, 16> palette = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		// Mode control register
		bool graphics = false; // 0: alpha, 1: graphics
		bool monochrome = false; // 1: Use mono display attributes
		bool extend8to9 = false; // For alpha 9px char width, extend 8th pixel for chars 0xC0-0xDF (mono/MDA emulation)
		bool blink = false; // 0: ATR3 = hi bg color, 1: blink
		bool pelPanCompatibility = false; // 1: Line-compare resets pelpan to 0, 0: line compare has no effect on pelpan
		bool pelWidth = false; // 1: 8 bits select a color (256 color mode 13h), 0: all other modes
		bool p4p5Select = false;

		// Overscan control register
		BYTE overscanColor = 0;

		// Color plane enable register
		BYTE colorPlaneEnable = 0x0F;
		BYTE videoStatusMux = 0;

		// Horizontal Pixel Panning register
		BYTE hPelPanning = 0;

		// Color Select Register
		BYTE colorSelect = 0;

		void Reset();
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

		void ConnectPorts();
		void DisconnectPorts();

		const AttrControllerData& GetData() const { return m_data; }

		void ResetMode() { m_currMode = RegisterMode::ADDRESS; }

		BYTE GetColor(BYTE index) const {
			m_lastDot = (m_data.palette[index & 15] | GetColor67());
			if (m_data.p4p5Select)
			{
				m_lastDot &= 0b11001111;
				m_lastDot |= GetColor45();
			}
			return m_lastDot;
		}
		BYTE GetLastDot() const { return m_lastDot; }

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		const WORD m_baseAddress;

		mutable BYTE m_lastDot = 0;

		uint32_t GetRGB32Color(BYTE value);

		BYTE GetColor67() const { return (m_data.colorSelect << 4) & 0b11000000; }
		BYTE GetColor45() const { return (m_data.colorSelect << 4) & 0b00110000; }

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
			ATTR_COLOR_SELECT = 0x14,

			_ATTR_MAX = ATTR_COLOR_SELECT,
			ATTR_INVALID = 0xFF
		} m_currRegister = AttrControllerAddress::ATTR_INVALID;

		BYTE ReadAddress();
		BYTE ReadData();
		void WriteData(BYTE value);

		AttrControllerData m_data;
	};
}
