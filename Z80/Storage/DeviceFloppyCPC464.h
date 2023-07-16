#pragma once

#include <CPU/CPUCommon.h>
#include "DeviceFloppy.h"
#include <vector>
#include <deque>

using emul::WORD;
using emul::BYTE;

namespace fdc
{
	class DeviceFloppyCPC464 : public DeviceFloppy
	{
	public:
		DeviceFloppyCPC464(WORD baseAddress, size_t clockSpeedHz);

		DeviceFloppyCPC464() = delete;
		DeviceFloppyCPC464(const DeviceFloppyCPC464&) = delete;
		DeviceFloppyCPC464& operator=(const DeviceFloppyCPC464&) = delete;
		DeviceFloppyCPC464(DeviceFloppyCPC464&&) = delete;
		DeviceFloppyCPC464& operator=(DeviceFloppyCPC464&&) = delete;

		virtual void Init() override;
		void Reset();

		virtual bool IsActive(BYTE drive) override { return true; };

	protected:
		BYTE Read();
		void Write(BYTE value);

		virtual void SetDMAPending() override { }
		virtual void SetInterruptPending() override { }

	};
}
