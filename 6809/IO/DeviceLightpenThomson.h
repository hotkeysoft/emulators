#pragma once

#include <IO/DeviceMouse.h>

namespace pia { class Device6520MO5_PIA; }
namespace video { class VideoThomson; }

namespace mouse
{
	class DeviceLightpenThomson : public DeviceMouse
	{
	public:
		DeviceLightpenThomson();

		virtual void Reset() override;

		void SetPIA(pia::Device6520MO5_PIA* pia) { assert(pia); m_pia = pia; }
		void SetVideo(video::VideoThomson* video) { assert(video); m_video = video; }

		virtual void SetButtonClick(int32_t x, int32_t y, int button, bool down) override;
		virtual void SetMouseMoveAbs(int32_t x, int32_t y) override;

	protected:
		pia::Device6520MO5_PIA* m_pia = nullptr;
		video::VideoThomson* m_video = nullptr;
	};
}

