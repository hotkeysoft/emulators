#pragma once

#include "Common.h"
#include "Memory.h"
#include "PortConnector.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace video
{
	class Video : public PortConnector
	{
	public:
		Video(uint16_t width, uint16_t height, float vScale = 1.0f);
		virtual ~Video();

		Video() = delete;
		Video(const Video&) = delete;
		Video& operator=(const Video&) = delete;
		Video(Video&&) = delete;
		Video& operator=(Video&&) = delete;

		virtual void Reset() {};
		virtual void Init(emul::Memory* memory, const char* charROM, BYTE border, bool forceMono = false);

		virtual void Tick() = 0;

		virtual bool IsMonoAdapter() { return false; }

		void RenderFrame(uint16_t w, uint16_t h, uint32_t borderRGB = 0xFF000000);

	protected:
		enum class MonitorType
		{
			RGB = 1,
			COMPOSITE,

			MONO = 0x80,
			MONO_WHITE,
			MONO_AMBER,
			MONO_GREEN,
		} m_monitor = MonitorType::RGB;

		void InitMonitor(bool forceMono);
		bool IsMonochromeMonitor() const { return (uint8_t)m_monitor & (uint8_t)MonitorType::MONO; }
		bool IsRGBMonitor() const { return m_monitor == MonitorType::RGB; }
		bool IsCompositeMonitor() const { return m_monitor == MonitorType::COMPOSITE; }

		const uint32_t* GetMonitorPalette() const { return m_monitorPalette; }

		// SDL
		SDL_Window* m_sdlWindow = nullptr;
		SDL_Renderer* m_sdlRenderer = nullptr;
		SDL_Texture* m_sdlTexture = nullptr;

		uint16_t m_sdlWidth = 0;
		uint16_t m_sdlHeight = 0;

		BYTE m_sdlBorderPixels;
		WORD m_sdlHBorder;
		WORD m_sdlVBorder;

		float m_vScale = 1.0f;

		const uint32_t* m_monitorPalette = nullptr;

		uint32_t* m_frameBuffer;
	};
}
