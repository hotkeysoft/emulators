#pragma once

#include "../Common.h"
#include "../CPU/Memory.h"
#include "Device6845.h"
#include "Video.h"

using emul::WORD;

namespace video
{
	// Base class for all 6845-based video cards (CGA, MDA, TGA, PCjr)
	//
	class Video6845 : public Video, public crtc::EventHandler
	{
	public:
		Video6845(WORD baseAddress, BYTE charWidth = 8);

		Video6845() = delete;
		Video6845(const Video6845&) = delete;
		Video6845& operator=(const Video6845&) = delete;
		Video6845(Video6845&&) = delete;
		Video6845& operator=(Video6845&&) = delete;

		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;
		virtual void Reset() override;
		virtual void Tick() override;

		virtual void EnableLog(SEVERITY minSev = LOG_INFO) override;

		// crtc::EventHandler
		virtual void OnRenderFrame() override;
		virtual void OnNewFrame() override;
		virtual void OnEndOfRow() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(json& from) override;

		virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;

	protected:
		const WORD m_baseAddress;

		virtual bool ConnectTo(emul::PortAggregator& dest) override;

		typedef void(Video6845::* DrawFunc)();
		void SetDrawFunc(DrawFunc func) { m_drawFunc = func; }

		bool IsCursor() const;

		typedef void(Video6845::* DrawFunc)();
		
		crtc::Device6845& GetCRTC() { return m_crtc; }
		const crtc::Device6845& GetCRTC() const { return m_crtc; }

	private:
		DrawFunc m_drawFunc = nullptr;

		crtc::Device6845 m_crtc;
	};
}
