#include "stdafx.h"
#include "VideoColecoVision.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	VideoColecoVision::VideoColecoVision() : Video(), Logger("vidColeco")
	{
	}

	void VideoColecoVision::EnableLog(SEVERITY minSev)
	{
		m_vdp.EnableLog(minSev);
		Video::EnableLog(minSev);
	}

	void VideoColecoVision::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(512, 262);

		// TODO: Need masking in io ports to simplify this

		// Coleco decodes io 0xA0-0xBF to VDP. VDP decodes A0
		const WORD basePort = 0xA0;
		for (WORD offset = 0; offset < 32; offset += 2)
		{
			Connect(basePort + offset + 0, static_cast<PortConnector::INFunction>(&VideoColecoVision::Read0));
			Connect(basePort + offset + 0, static_cast<PortConnector::OUTFunction>(&VideoColecoVision::Write0));

			Connect(basePort + offset + 1, static_cast<PortConnector::INFunction>(&VideoColecoVision::Read1));
			Connect(basePort + offset + 1, static_cast<PortConnector::OUTFunction>(&VideoColecoVision::Write1));
		}

		m_vdp.Init(this);
		m_vdp.Reset();
	}

	SDL_Rect VideoColecoVision::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		return SDL_Rect{ 0, 0, 342, 262 };
	}

	void VideoColecoVision::Serialize(json& to)
	{
		Video::Serialize(to);
		m_vdp.Serialize(to["vdp"]);
	}

	void VideoColecoVision::Deserialize(const json& from)
	{
		Video::Deserialize(from);
		m_vdp.Deserialize(from["vdp"]);
	}
}