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
	}

	void DeviceFloppy::Reset()
	{
		LogPrintf(LOG_INFO, "Reset");
		
		Logger::SEVERITY oldSev = GetLogLevel();
		EnableLog(LOG_OFF);

		m_diskLoaded = false;
		m_diskChanged = false;
		m_motorEnabled = false;
		m_motorSpeed = DEFAULT_RPM;
		m_ticksPerRotation = UINT32_MAX;
		m_motorPulseCounter = UINT32_MAX;
		m_motorPulse = false;
		m_stepDelay = DEFAULT_STEP_MS;
		m_ticksPerTrack = UINT32_MAX;
		m_seekCounter = UINT32_MAX;
		m_isSeeking = false;
		m_stepDirection = StepDirection::OUTER;
		m_maxTrack = MAX_TRACK;
		m_currTrack = 0;
		m_currSector = 0;
		m_headCount = DEFAULT_HEAD_COUNT;
		m_currHead = 0;

		SetMotorSpeed(DEFAULT_RPM);
		SetStepDelay(DEFAULT_STEP_MS);
		SetHeadCount(DEFAULT_HEAD_COUNT);
		SelectHead(0);

		ClearDiskChanged();

		EnableLog(oldSev);
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

	void DeviceFloppy::SetMotorSpeed(WORD rpm) 
	{ 
		LogPrintf(LOG_INFO, "Set Motor Speed: %d rpm", rpm);
		m_motorSpeed = std::clamp(rpm, MIN_RPM, MAX_RPM);
		if (m_motorSpeed != rpm)
		{
			LogPrintf(LOG_WARNING, "Motor Speed clamped to %d rpm", m_motorSpeed);
		}
		m_ticksPerRotation = m_clockSpeed / m_motorSpeed / 2;
		LogPrintf(LOG_DEBUG, "Ticks per Rotation: %d", m_ticksPerRotation);

		if (m_motorPulseCounter > m_ticksPerRotation)
		{
			ResetPulseCounter();
		}
	}

	void DeviceFloppy::SetHeadCount(WORD heads)
	{
		LogPrintf(LOG_INFO, "Set Head Count: %d", heads);
		m_headCount = std::clamp(heads, MIN_HEADS, MAX_HEADS);
		if (m_headCount != heads)
		{
			LogPrintf(LOG_WARNING, "Head Count clamped to %d", m_headCount);
		}
		m_currHead = 0;
	}
	// Heads are numbered [0..HeadCount-1]
	void DeviceFloppy::SelectHead(WORD head)
	{
		LogPrintf(LOG_INFO, "Select Head: %d", head);
		if (head >= m_headCount)
		{
			LogPrintf(LOG_ERROR, "Invalid Head: %d (max = %d)", head, m_headCount - 1);
			m_currHead = 0;
		}
		else
		{
			m_currHead = head;
		}
	}

	void DeviceFloppy::SetStepDelay(WORD millis)
	{
		LogPrintf(LOG_INFO, "Set Step Delay: %d ms", millis);
		m_stepDelay = std::clamp(millis, MIN_STEP_MS, MAX_STEP_MS);
		if (m_stepDelay != millis)
		{
			LogPrintf(LOG_WARNING, "Step Delay clamped to %d ms", m_stepDelay);
		}

		m_ticksPerTrack = DelayToTicks(m_stepDelay);
		LogPrintf(LOG_DEBUG, "Ticks per Track: %d", m_ticksPerTrack);

		if (m_seekCounter > m_ticksPerTrack)
		{
			ResetSeekCounter();
		}
	}

	void DeviceFloppy::Step()
	{

	}

	void DeviceFloppy::Serialize(json& to)
	{

	}
	void DeviceFloppy::Deserialize(const json& from)
	{

	}

}