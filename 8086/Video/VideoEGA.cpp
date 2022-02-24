#include "VideoEGA.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;
using emul::ADDRESS;

namespace video
{
	VideoEGA::VideoEGA(RAMSIZE ramsize, WORD baseAddressMisc, WORD baseAddressMono, WORD baseAddressColor) :
		Logger("EGA"),
		m_ramSize(ramsize),
		m_baseAddressMisc(baseAddressMisc),
		m_baseAddressMono(baseAddressMono),
		m_baseAddressColor(baseAddressColor),
		m_egaROM("EGABIOS", 16384, emul::MemoryType::ROM)
	{
	}

	void VideoEGA::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		// charROM not used, part of the BIOS

		m_egaROM.LoadFromFile("data/XT/EGA_6277356_C0000.BIN");
		memory->Allocate(&m_egaROM, 0xC0000);

		// Mode Control Register
		//Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&VideoCGA::WriteModeControlRegister));

		//// Color Select Register
		//Connect(m_baseAddress + 9, static_cast<PortConnector::OUTFunction>(&VideoCGA::WriteColorSelectRegister));

		//// Status Register
		//Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&VideoCGA::ReadStatusRegister));

		Video::Init(memory, charROM, forceMono);

		// Normally max y is nnn but leave some room for custom crtc programming
		InitFrameBuffer(2048, 400);

		AddMode("text", (DrawFunc)&VideoEGA::DrawTextMode, (AddressFunc)&VideoEGA::GetBaseAddress, (ColorFunc)&VideoEGA::GetIndexedColor);
		SetMode("text");
	}	

	SDL_Rect VideoEGA::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		return SDL_Rect{ 0, 0, 640, 200 };
	}

	void VideoEGA::Tick()
	{
	}

	void VideoEGA::DrawTextMode()
	{
		LogPrintf(LOG_DEBUG, "DrawTextMode");
	}

	void VideoEGA::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddressMisc;
		to["id"] = "ega";
	}

	void VideoEGA::Deserialize(json& from)
	{
		if (from["baseAddress"] != m_baseAddressMisc)
		{
			throw emul::SerializableException("VideoEGA: Incompatible baseAddress");
		}

		if (from["id"] != "ega")
		{
			throw emul::SerializableException("VideoEGA: Incompatible mode");
		}

		//OnChangeMode();
		//OnNewFrame();
	}
}
