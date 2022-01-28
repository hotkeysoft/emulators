#include "DeviceSN76489.h"

#include <assert.h>

using emul::GetHByte;
using emul::GetLByte;
using emul::SetLByte;
using emul::SetHByte;

namespace sn76489
{
	void Voice::SetAttenuation(BYTE value)
	{
		value &= 0b1111;
		LogPrintf(LOG_DEBUG, "SetAttenuation, value=%02Xh", value);
		m_attenuation = value;
	}

	void Voice::Serialize(json& to)
	{
		to["n"] = m_n;
		to["counter"] = m_counter;
		to["attenuation"] = m_attenuation;
		to["out"] = m_out;
	}

	void Voice::Deserialize(json& from)
	{
		m_n = from["n"];
		m_counter = from["counter"];
		m_attenuation = from["attenuation"];
		m_out = from["out"];
	}

	// =================================

	VoiceSquare::VoiceSquare(const char* label) : Voice(label)
	{
	}

	void VoiceSquare::Tick()
	{
		if (m_n == 0 || m_n == 1)
		{
			SetOutput(true);
			return;
		}

		if (--m_counter == 0)
		{
			//LogPrintf(LOG_DEBUG, "ToggleOutput");
			m_counter = m_n;
			ToggleOutput();
		}
	}

	void VoiceSquare::SetData(BYTE value, bool highLow)
	{
		LogPrintf(LOG_DEBUG, "SetData, value=%02Xh", value);
		if (highLow)
		{
			m_n &= 0b0000001111;
			m_n |= ((WORD)(value & 0b111111) << 4);
		}
		else
		{
			m_n &= 0b1111110000;
			m_n |= (value & 0b1111);
		}
	}

	// =================================

	VoiceNoise::VoiceNoise(const char* label, Voice* trigger) : 
		Voice(label),
		m_trigger(trigger)
	{
		assert(trigger);
		ResetShiftRegister();
		SetOutput(false);
	}

	void VoiceNoise::Tick()
	{	
		// voice 2 modulated
		if (m_n == 0)
		{
			bool trigger = m_trigger->GetRawOutput();
			if (!m_lastTrigger && trigger)
			{
				Shift();
			}
			m_lastTrigger = trigger;
		}
		else if (--m_counter == 0)
		{
			m_counter = m_n;
			m_internalOutput = !m_internalOutput;
			if (m_internalOutput)
			{
				Shift();
			}
		}
	}

	static bool parity(WORD val)
	{
		val ^= (val >> 8);
		val ^= (val >> 4);
		val ^= (val >> 2);
		val ^= (val >> 1);
		return val & 1;
	}

	void VoiceNoise::Shift()
	{
		bool input;
		if (m_whiteNoise)
		{
			input = parity(m_shiftRegister & m_noisePattern);
		}
		else
		{
			input = (m_shiftRegister & 1);
		}

		m_shiftRegister = (m_shiftRegister >> 1);
		m_shiftRegister |= (input << (m_shiftRegisterLen - 1));

		SetOutput(m_shiftRegister & 1);
	}

	void VoiceNoise::SetData(BYTE value, bool)
	{
		LogPrintf(LOG_DEBUG, "SetData, value=%02Xh", value);

		m_whiteNoise = value & 0b100;

		BYTE shiftRate = value & 0b11;
		switch (shiftRate)
		{
		case 0: m_n = 0x10; break;
		case 1: m_n = 0x20; break;
		case 2: m_n = 0x40; break;
		case 3: m_n = 0; break; // Tone2
		}
		m_counter = m_n;

		LogPrintf(LOG_INFO, "SetData, mode=[%s] counter=[%02Xh]",
			m_whiteNoise ? "White Noise" : "Periodic",
			m_n);

		ResetShiftRegister();
	}

	void VoiceNoise::Serialize(json& to)
	{
		Voice::Serialize(to);

		to["lastTrigger"] = m_lastTrigger;
		to["whiteNoise"] = m_whiteNoise;
		to["internalOutput"] = m_internalOutput;
		to["shiftRegister"] = m_shiftRegister;
	}
	void VoiceNoise::Deserialize(json& from)
	{
		Voice::Deserialize(from);

		m_lastTrigger = from["lastTrigger"];
		m_whiteNoise = from["whiteNoise"];
		m_internalOutput = from["internalOutput"];
		m_shiftRegister = from["shiftRegister"];
	}

	// =================================

	DeviceSN76489::DeviceSN76489(WORD baseAddress, size_t clockSpeedHz) :
		Logger("SN76489"),
		m_baseAddress(baseAddress)
	{
		m_voices[0] = new VoiceSquare("SN76489_V0");
		m_voices[1] = new VoiceSquare("SN76489_V1");
		m_voices[2] = new VoiceSquare("SN76489_V2");
		m_voices[3] = new VoiceNoise("SN76489_N", m_voices[2]);
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

	void DeviceSN76489::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		for (int i = 0; i < 4; ++i)
		{
			m_voices[i]->EnableLog(minSev);
		}
	}

	void DeviceSN76489::Reset()
	{
	}

	void DeviceSN76489::Init()
	{
		for (int i = 0; i < 8; ++i)
		{
			Connect(m_baseAddress + i, static_cast<PortConnector::OUTFunction>(&DeviceSN76489::WriteData));
		}

		for (int i = 0; i < 4; ++i)
		{
			m_voices[i]->Init();
		}
	}

	void DeviceSN76489::WriteData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteData, value=%02Xh", value);

		if (!(value & 0x80)) // Data command, uses currently last register as destination
		{
			if (m_currFunc == Function::DATA)
			{
				m_currDest->SetData(value, true);
			}
		}
		else // Latch/Data command
		{
			// Save register for subsequent Data commands
			m_currDest = m_voices[(value >> 5) & 3];
			m_currFunc = (value & 0b00010000) ? Function::VOL : Function::DATA;
			
			switch (m_currFunc)
			{
			case Function::VOL: m_currDest->SetAttenuation(value); break;
			case Function::DATA: m_currDest->SetData(value, false); break;
			}
		}

		m_ready = 32;
	}

	void DeviceSN76489::Tick()
	{
		if (m_ready)
		{
			--m_ready;
		}

		static WORD cooldown = m_tickDivider;
		if (--cooldown != 0)
			return;
		cooldown = m_tickDivider;

		for (int i = 0; i < 4; ++i)
		{
			m_voices[i]->Tick();
		}
	}

	WORD DeviceSN76489::GetOutput()
	{
		WORD out = 0;
		for (int i = 0; i < 4; ++i)
		{
			out += m_voices[i]->GetOutput();
		}
		return out;
	}

	void DeviceSN76489::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;

		to["ready"] = m_ready;
		to["function"] = m_currFunc;

		if (m_currDest == nullptr)
		{
			to["currDest"] = -1;
		}
		else for (int i = 0; i < 4; ++i)
		{
			if (m_currDest == m_voices[i])
			{
				to["currDest"] = i;
			}
		}

		m_voices[0]->Serialize(to["voice0"]);
		m_voices[1]->Serialize(to["voice1"]);
		m_voices[2]->Serialize(to["voice2"]);
		m_voices[3]->Serialize(to["noise"]);
	}

	void DeviceSN76489::Deserialize(json& from)
	{
		WORD baseAddress = from["baseAddress"];
		if (baseAddress != m_baseAddress)
		{
			throw emul::SerializableException("DeviceSN76489: Incompatible baseAddress");
		}

		m_ready = from["ready"];
		m_currFunc = from["function"];

		int currDest = from["currDest"];
		if (currDest >= 0 && currDest <= 3)
		{
			m_currDest = m_voices[currDest];
		}
		else
		{
			m_currDest = nullptr;
		}

		m_voices[0]->Deserialize(from["voice0"]);
		m_voices[1]->Deserialize(from["voice1"]);
		m_voices[2]->Deserialize(from["voice2"]);
		m_voices[3]->Deserialize(from["noise"]);
	}


}
