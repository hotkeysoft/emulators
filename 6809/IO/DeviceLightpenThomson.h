#pragma once

#include <IO/DeviceMouse.h>

namespace pia::thomson { class DevicePIAThomson; }
namespace video { class VideoThomson; }

namespace mouse
{
	class DeviceLightpenThomson : public DeviceMouse
	{
	public:
		DeviceLightpenThomson();

		virtual void Reset() override;

		void SetPIA(pia::thomson::DevicePIAThomson* pia) { assert(pia); m_pia = pia; }

		void SetVideo(video::VideoThomson* video) { assert(video); m_video = video; }

		virtual void SetButtonClick(int32_t x, int32_t y, int button, bool down) override;
		virtual void SetMouseMoveAbs(int32_t x, int32_t y) override;

	protected:
		pia::thomson::DevicePIAThomson* m_pia = nullptr;

		video::VideoThomson* m_video = nullptr;
	};
}

