#include "stdafx.h"

#include "DeviceFloppy.h"
#include <array>
#include <FileUtil.h>

using hscommon::fileUtil::File;
using emul::GetBit;

namespace fdd
{
	DeviceFloppy::DeviceFloppy(uint32_t clockSpeedHz) :
		Logger("fdd"),
		m_clockSpeed(clockSpeedHz)
	{
		SetMotorRPM(DEFAULT_RPM);
	}

	void DeviceFloppy::Reset()
	{
		m_currOpWait = 0;

		m_diskLoaded = false;
		m_diskChanged = false;
		m_motorEnabled = false;
		m_motorSpeed = DEFAULT_RPM;
		m_ticksPerPulse = UINT32_MAX;
		m_motorPulseCounter = UINT32_MAX;
		m_motorPulse = false;
		m_isSeeking = false;
		m_track = 0;
		m_sector = 0;
		m_head = 0;

		ClearDiskChanged();
	}

	void DeviceFloppy::Init()
	{
	}

	void DeviceFloppy::Tick()
	{
		if (!m_motorEnabled)
			return;

		--m_motorPulseCounter;
		if (m_motorPulseCounter == 0)
		{
			ResetPulseCounter();
			m_motorPulse = !m_motorPulse;
		}
	}

	void DeviceFloppy::EnableMotor(bool enable)
	{
		if (enable && !m_motorEnabled)
		{
			ResetPulseCounter();
		}
		m_motorEnabled = enable;
	}

	void DeviceFloppy::SetMotorRPM(WORD speed) 
	{ 
		speed = std::clamp(speed, MIN_RPM, MAX_RPM);
		m_motorSpeed = speed; 
		m_ticksPerPulse = m_clockSpeed / speed / 2;
		if (m_motorPulseCounter > m_ticksPerPulse)
		{
			ResetPulseCounter();
		}
	}

	uint32_t DeviceFloppy::DelayToTicks(uint32_t delayUS)
	{
		return delayUS * m_clockSpeed / 1000000;
	}

	void DeviceFloppy::Serialize(json& to)
	{

	}
	void DeviceFloppy::Deserialize(const json& from)
	{

	}

}