#include "stdafx.h"

#include "DeviceFloppy.h"
#include <array>
#include <FileUtil.h>

using hscommon::fileUtil::File;
using emul::GetBit;

namespace fdd
{
	const char* GetStepDirectionStr(StepDirection dir)
	{
		switch (dir)
		{
		case StepDirection::OUTER: return "OUTER";
		case StepDirection::INNER: return "INNER";
		default:
			NODEFAULT;
		}
	}

	DeviceFloppy::DeviceFloppy(uint32_t clockSpeedHz, bool connected) :
		Logger("fdd"),
		m_clockSpeed(clockSpeedHz),
		m_connected(connected)
	{
	}

	void DeviceFloppy::Reset()
	{
		LogPrintf(LOG_INFO, "Reset");
		LogPrintf(LOG_INFO, "Clock Speed = %d, connected = %d", m_clockSpeed, m_connected);
		
		Logger::SEVERITY oldSev = GetLogLevel();
		EnableLog(LOG_OFF);

		m_diskLoaded = false;
		m_diskChanged = false;
		m_motorEnabled = false;
		m_motorPulseCounter = UINT32_MAX;
		m_motorPulse = false;
		m_seekCounter = UINT32_MAX;
		m_isSeeking = false;
		m_isCalibrating = false;
		m_stepDirection = StepDirection::INNER;
		m_currTrack = 0;
		m_currSector = 0;
		m_currHead = 0;

		ClearDiskChanged();

		EnableLog(oldSev);
	}

	void DeviceFloppy::Tick()
	{
		if (!m_connected)
			return;

		if (m_motorEnabled && (--m_motorPulseCounter == 0))
		{
			ResetPulseCounter();
			m_motorPulse = !m_motorPulse;
		}

		if (m_isSeeking && (--m_seekCounter == 0))
		{
			ResetSeekCounter();
			m_isSeeking = false;
			int newTrack = m_currTrack + (int)m_stepDirection;
			// Allow one more inner track to signify "out of bounds"
			m_currTrack = std::clamp(newTrack, 0, (int)m_trackCount); 
			if (newTrack != m_currTrack)
			{
				LogPrintf(LOG_WARNING, "Seek past endpoint");
			}

			LogPrintf(LOG_INFO, "Seek End, new track: [%d]", m_currTrack);
		}
	}

	void DeviceFloppy::EnableMotor(bool enable)
	{
		if (!m_connected)
			return;

		LogPrintf(LOG_INFO, "Motor: %s", enable ? "ON" : "OFF");

		if (enable && !m_motorEnabled)
		{
			ResetPulseCounter();
		}
		m_motorEnabled = enable;
	}

	void DeviceFloppy::SetMotorSpeed(WORD rpm) 
	{ 
		if (!m_connected)
			return;

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
		if (!m_connected)
			return;

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
		if (!m_connected)
			return;

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

	void DeviceFloppy::SetTrackCount(WORD tracks)
	{
		if (!m_connected)
			return;

		LogPrintf(LOG_INFO, "Set Track Count: %d", tracks);
		m_trackCount = std::clamp(tracks, MIN_TRACKS, MAX_TRACKS);
		if (m_trackCount != tracks)
		{
			LogPrintf(LOG_WARNING, "Track Count clamped to %d", m_headCount);
		}
		m_currTrack = 0;
	}

	void DeviceFloppy::SetStepDelay(WORD millis)
	{
		if (!m_connected)
			return;

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

	void DeviceFloppy::SetStepDirection(StepDirection dir)
	{ 
		if (!m_connected)
			return;

		LogPrintf(LOG_INFO, "Set Step Direction: %s", GetStepDirectionStr(dir));
		m_stepDirection = dir; 
	}

	void DeviceFloppy::Step()
	{
		if (!m_connected)
			return;

		LogPrintf(LOG_INFO, "Step one track in [%s] direction", GetStepDirectionStr(m_stepDirection));

		if (m_isSeeking)
		{
			LogPrintf(LOG_WARNING, "Step: Already seeking, ignored");
			return;
		}
		else if ((m_currTrack == 0) && (m_stepDirection == StepDirection::OUTER))
		{
			LogPrintf(LOG_WARNING, "Step: Already at track 0, ignored");
			return;
		}

		m_isSeeking = true;
		ResetSeekCounter();
	}

	void DeviceFloppy::Serialize(json& to)
	{

	}
	void DeviceFloppy::Deserialize(const json& from)
	{

	}

}