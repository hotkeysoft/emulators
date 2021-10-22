#include "DeviceSN76489.h"

using emul::GetHByte;
using emul::GetLByte;
using emul::SetLByte;
using emul::SetHByte;

namespace sn76489
{
	void Voice::SetAttenuation(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "SetAttenuation, value=%02Xh", value);
		m_attenuation = value;
	}

	// =================================

	VoiceSquare::VoiceSquare(const char* label) : Voice(label)
	{
	}

	void VoiceSquare::Tick()
	{
		static WORD cooldown = m_tickDivider;
		if (--cooldown != 0)
			return;
		cooldown = m_tickDivider;

		if (--m_counter == 0)
		{
			//LogPrintf(LOG_DEBUG, "ToggleOutput");
			m_counter = m_n;
			ToggleOutput();
		}
	}

	void VoiceSquare::SetLowData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "SetLowFrequency, value=%02Xh", value);
		m_n &= 0b1111110000;
		m_n |= value;
	}

	void VoiceSquare::SetHighData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "SetHighFrequency, value=%02Xh", value);
		m_n &= 0b0000001111;
		m_n |= ((WORD)value << 4);
	}

	// =================================

	VoiceNoise::VoiceNoise(const char* label) : Voice(label)
	{
	}

	void VoiceNoise::Tick()
	{
	}

	void VoiceNoise::SetLowData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "SetNoiseData, value=%02Xh", value);
	}

	// =================================

	DeviceSN76489::DeviceSN76489(WORD baseAddress, size_t clockSpeedHz) :
		Logger("SN76489"),
		m_baseAddress(baseAddress)
	{		
		m_voices[0] = new VoiceSquare("SN76489_V0");
		m_voices[1] = new VoiceSquare("SN76489_V1");
		m_voices[2] = new VoiceSquare("SN76489_V2");
		m_voices[3] = new VoiceNoise("SN76489_NOISE");
		m_currDest = m_voices[0];

		s_clockSpeed = clockSpeedHz;

		Reset();
	}

	DeviceSN76489::~DeviceSN76489()
	{
		for (int i = 0; i < 4; ++i)
		{
			delete m_voices[i];
		}
	}

	void DeviceSN76489::EnableLog(bool enable, SEVERITY minSev)
	{
		Logger::EnableLog(enable, minSev);
		for (int i = 0; i < 4; ++i)
		{
			m_voices[i]->EnableLog(enable, minSev);
		}
	}

	void DeviceSN76489::Reset()
	{
	}

	void DeviceSN76489::Init()
	{
		Connect(m_baseAddress, static_cast<PortConnector::OUTFunction>(&DeviceSN76489::WriteData));

		for (int i = 0; i < 4; ++i)
		{
			m_voices[i]->Init();
		}
	}

	void DeviceSN76489::WriteData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteData, value=%02Xh", value);

		if (!(value & 0x80))
		{
			m_currDest->SetHighData(value);
		}
		else
		{
			m_currDest = m_voices[(value >> 5) & 3];
			bool select = value & 0b00010000;
			value &= 0b1111;
			if (select)
			{
				m_currDest->SetAttenuation(value);
			}
			else
			{
				m_currDest->SetLowData(value);
			}
		}
	}

	void DeviceSN76489::Tick()
	{
		for (int i = 0; i < 4; ++i)
		{
			m_voices[i]->Tick();
		}
	}

	BYTE DeviceSN76489::GetOutput()
	{
		BYTE out = 0;
		for (int i = 0; i < 4; ++i)
		{
			out += m_voices[i]->GetOutput();
		}
		return out;
	}

}
