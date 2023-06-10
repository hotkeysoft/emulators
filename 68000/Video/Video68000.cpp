#include "stdafx.h"
#include "Video68000.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	Video68000::Video68000() : Video(), Logger("vid68000")
	{

	}

	void Video68000::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(320, 200);
	}

	void Video68000::Tick()
	{
	}

	SDL_Rect Video68000::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		return SDL_Rect{ 0, 0, 320, 200 };
	}
}