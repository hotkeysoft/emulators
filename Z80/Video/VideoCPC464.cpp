#include "stdafx.h"
#include "VideoCPC464.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	VideoCPC464::VideoCPC464() : Video(), Logger("vidCPC464")
	{
	}

	void VideoCPC464::EnableLog(SEVERITY minSev)
	{
//		m_vdp.EnableLog(minSev);
		Video::EnableLog(minSev);
	}

	void VideoCPC464::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(512, 262);

		Connect("01xxxxxx", static_cast<PortConnector::OUTFunction>(&VideoCPC464::Write));
	}

	SDL_Rect VideoCPC464::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		border = 8;
		SDL_Rect rect { 0, 0, 320, 200 };//m_vdp.GetDisplayRect();
		rect.x -= border;
		rect.y -= border;
		rect.w += border * 2;
		rect.h += border * 2;

		return rect;
	}

	void VideoCPC464::Write(BYTE value)
	{
		LogPrintf(LOG_TRACE, "Write(%04x, %02x)", GetCurrentPort(), value);

		switch (value >> 6)
		{
		case 0:
			if (GetBit(value, 4))
			{
				LogPrintf(LOG_INFO, "Select pen: [border]");
			}
			else
			{
				LogPrintf(LOG_INFO, "Select pen: [%d]", (value & 15));
			}
			break;
		case 1:
			LogPrintf(LOG_INFO, "Select color: [%d]", (value & 31));
			break;
		case 2:
			LogPrintf(LOG_INFO, "[%cINT] [%cHROM] [%cLROM] [MODE%d]",
				(GetBit(value, 4) ? ' ' : '/'),
				(GetBit(value, 3) ? '/' : ' '),
				(GetBit(value, 2) ? '/' : ' '),
				(value & 3));
			break;
		case 3:
			LogPrintf(LOG_INFO, "(Function 3, not present on CPC464)");
			break;
		}
	}

	void VideoCPC464::Serialize(json& to)
	{
		Video::Serialize(to);
	}

	void VideoCPC464::Deserialize(const json& from)
	{
		Video::Deserialize(from);
	}
}