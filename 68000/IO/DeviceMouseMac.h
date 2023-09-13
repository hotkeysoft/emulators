#pragma once

#include <IO/DeviceMouse.h>
#include <deque>

namespace via::mac { class Device6522Mac; }
namespace scc { class Device8530; }

namespace mouse::mac
{
	class DeviceMouseMac: public DeviceMouse
	{
	public:
		DeviceMouseMac();

		virtual void Reset() override;

		void SetVIA(via::mac::Device6522Mac* via) { assert(via); m_via = via; }
		void SetSCC(scc::Device8530* scc) { assert(scc); m_scc = scc; }

		virtual void SetButtonClick(int32_t x, int32_t y, int button, bool down) override;
		virtual void SetMouseMoveRel(int32_t x, int32_t y) override;

		void Tick();

	protected:
		struct MouseData
		{
			int dx = 0;
			int dy = 0;
		};
		std::deque<MouseData> m_mouseQueue;

		void PlotLine(int x0, int y0, int x1, int y1);

		via::mac::Device6522Mac* m_via = nullptr;
		scc::Device8530* m_scc = nullptr;
	};
}
