#pragma once

#include <CPU/CPUCommon.h>
#include <map>

using emul::BYTE;

namespace mouse
{
	class DeviceMouse : virtual public Logger
	{
	public:
		DeviceMouse() : Logger("mouse") {}
		virtual ~DeviceMouse() {}

		DeviceMouse(const DeviceMouse&) = delete;
		DeviceMouse& operator=(const DeviceMouse&) = delete;
		DeviceMouse(DeviceMouse&&) = delete;
		DeviceMouse& operator=(DeviceMouse&&) = delete;

		virtual void Init() {};
		virtual void Reset() {};

		virtual void SetButtonClick(int32_t x, int32_t y, int button, bool down) {}
		virtual void SetMouseMoveAbs(int32_t x, int32_t y) {}
		virtual void SetMouseMoveRel(int32_t x, int32_t y) {}
	};
}
