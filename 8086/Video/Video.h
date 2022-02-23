#pragma once

#include "../Common.h"
#include "../Serializable.h"
#include "../CPU/Memory.h"
#include "../CPU/PortConnector.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

#include <SDL_rect.h>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace video
{
	class Renderer
	{
	public:
		virtual void Render() = 0;
	};

	class Video : public PortConnector, public emul::Serializable
	{
	public:
		Video();
		virtual ~Video();

		Video(const Video&) = delete;
		Video& operator=(const Video&) = delete;
		Video(Video&&) = delete;
		Video& operator=(Video&&) = delete;

		virtual void Reset() {};
		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false);

		virtual void Tick() = 0;

		virtual bool IsMonoAdapter() { return false; }

		void RenderFrame();

		virtual void Serialize(json& to) = 0;
		virtual void Deserialize(json& from) = 0;

		void AddRenderer(Renderer* r) { m_renderers.push_back(r); }

		SDL_Window* GetWindow() const { return m_sdlWindow; }
		SDL_Renderer* GetRenderer() const { return m_sdlRenderer; }

		virtual uint32_t GetBackgroundColor() const { return 0; }
		virtual uint32_t GetIndexedColor(BYTE index) const { return index; }
		virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const = 0;

		void SetBorder(BYTE border) { m_border = border; }

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

		uint16_t m_sdlWidth = 0;
		uint16_t m_sdlHeight = 0;

		BYTE m_border = 0;

		const uint32_t* m_monitorPalette = nullptr;

		// Framebuffer
		void BeginFrame();
		void NewLine();
		void DrawPixel(uint32_t color) { m_fb[m_fbCurrPos++] = color; }
		void DrawBackground(BYTE width);
		void DrawBackground(BYTE width, uint32_t color);

		// Init frame buffer and associated sdl sdl texture
		virtual void InitFrameBuffer(WORD width, WORD height);

		// Clear framebuffer data and associated sdl texture
		void DestroyFrameBuffer();

		std::vector<uint32_t> m_fb;
		uint32_t m_fbCurrPos = 0;
		uint32_t m_fbCurrY = 0;
		uint32_t m_fbWidth = 0;
		uint32_t m_fbHeight = 0;

		SDL_Texture* m_sdlTexture = nullptr;

		std::vector<Renderer*> m_renderers;
	};
}
