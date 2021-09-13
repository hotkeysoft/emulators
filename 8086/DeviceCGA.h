#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Logger.h"
#include "MemoryBlock.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

struct SDL_Window;
struct SDL_Renderer;

namespace cga
{
	class DeviceCGA : public PortConnector
	{
	public:
		DeviceCGA(WORD baseAddress);

		DeviceCGA() = delete;
		DeviceCGA(const DeviceCGA&) = delete;
		DeviceCGA& operator=(const DeviceCGA&) = delete;
		DeviceCGA(DeviceCGA&&) = delete;
		DeviceCGA& operator=(DeviceCGA&&) = delete;

		void Init(const char* charROM);
		void Reset();

		void Tick();

		BYTE IN();
		void OUT(BYTE value);

		BYTE ReadStatus();

		emul::MemoryBlock& GetVideoRAM() { return m_screenB800; }

	protected:
		const WORD m_baseAddress;

		bool IsHSync() { return m_hPos > 640; }
		bool IsVSync() { return m_vPos > 200; }

		WORD m_hPos = 0;
		WORD m_vPos = 0;

		// 16K screen buffer
		emul::MemoryBlock m_screenB800;
		emul::MemoryBlock m_charROM;
		BYTE* m_charROMStart;

		// SDL
		SDL_Window* m_sdlWindow = nullptr;
		SDL_Renderer* m_sdlRenderer = nullptr;

		void Render();
	};
}
