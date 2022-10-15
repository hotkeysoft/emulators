#include "stdafx.h"

#include "SoundVIC.h"

using emul::GetBit;
using emul::GetMSB;

namespace sound::vic
{
	void Voice::Reset()
	{
		m_n = 0xFF;
		m_counter = 0xFF;
	}

	void Voice::Serialize(json& to)
	{
		to["enabled"] = m_enabled;
		to["n"] = m_n;
		to["counter"] = m_counter;
		to["out"] = m_out;
	}

	void Voice::Deserialize(const json& from)
	{
		m_enabled = from["enabled"];
		m_n = from["n"];
		m_counter = from["counter"];
		m_out = from["out"];
	}

	void Voice::SetFrequency(BYTE value)
	{
		m_n = 127 - (value & 127);
		m_enabled = GetMSB(value);
	}

	// =================================

	VoiceSquare::VoiceSquare(const char* label) : Voice(label)
	{
	}

	void VoiceSquare::Tick()
	{
		if (--m_counter == 0)
		{
			m_counter = m_n;
			ToggleOutput();
		}
	}

	// =================================

	VoiceNoise::VoiceNoise(const char* label) : Voice(label)
	{
		ResetShiftRegister();
		SetOutput(false);
	}

	void VoiceNoise::Tick()
	{
		if (--m_counter == 0)
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
		//if (m_whiteNoise)
		//{
			input = parity(m_shiftRegister & m_noisePattern);
		//}
		//else
		{
//			input = (m_shiftRegister & 1);
		}

		m_shiftRegister = (m_shiftRegister >> 1);
		m_shiftRegister |= (input << (m_shiftRegisterLen - 1));

		SetOutput(m_shiftRegister & 1);
	}

	void VoiceNoise::Serialize(json& to)
	{
		Voice::Serialize(to);

		to["internalOutput"] = m_internalOutput;
		to["shiftRegister"] = m_shiftRegister;
	}
	void VoiceNoise::Deserialize(const json& from)
	{
		Voice::Deserialize(from);

		m_internalOutput = from["internalOutput"];
		m_shiftRegister = from["shiftRegister"];
	}

	// =================================

	SoundVIC::SoundVIC() : Logger("soundVIC")
	{
		m_voices[0] = new VoiceSquare("vic_V0");
		m_voices[1] = new VoiceSquare("vic_V1");
		m_voices[2] = new VoiceSquare("vic_V2");
		m_voices[3] = new VoiceNoise("vic_N");

		Reset();
	}

	SoundVIC::~SoundVIC()
	{
		for (int i = 0; i < 4; ++i)
		{
			delete m_voices[i];
		}
	}

	void SoundVIC::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		for (int i = 0; i < 4; ++i)
		{
			m_voices[i]->EnableLog(minSev);
		}
	}

	void SoundVIC::Reset()
	{
		m_counter = 0;
		for (int i = 0; i < 4; ++i)
		{
			m_voices[i]->Reset();
		}
	}

	void SoundVIC::Init()
	{
		Reset();
		for (int i = 0; i < 4; ++i)
		{
			m_voices[i]->Init();
		}
	}

	void SoundVIC::Tick()
	{
		const unsigned int lastCounter = m_counter++;
		const unsigned int changed = m_counter ^ lastCounter;
		if (GetBit(changed, 7)) m_voices[0]->Tick(); // CLK/256
		if (GetBit(changed, 6)) m_voices[1]->Tick(); // CLK/128
		if (GetBit(changed, 5)) m_voices[2]->Tick(); // CLK/64
		if (GetBit(changed, 4)) m_voices[3]->Tick(); // CLK/32
	}

	WORD SoundVIC::GetOutput()
	{
		WORD out = 0;
		for (int i = 0; i < 4; ++i)
		{
			out += m_voices[i]->GetOutput() ? 1 : 0;
		}
		return out * m_volume * 1024;
	}

	void SoundVIC::Serialize(json& to)
	{
		to["volume"] = m_volume;
		to["counter"] = m_counter;

		m_voices[0]->Serialize(to["voice0"]);
		m_voices[1]->Serialize(to["voice1"]);
		m_voices[2]->Serialize(to["voice2"]);
		m_voices[3]->Serialize(to["noise"]);
	}

	void SoundVIC::Deserialize(const json& from)
	{
		m_volume = from["volume"];
		m_counter = from["counter"];

		m_voices[0]->Deserialize(from["voice0"]);
		m_voices[1]->Deserialize(from["voice1"]);
		m_voices[2]->Deserialize(from["voice2"]);
		m_voices[3]->Deserialize(from["noise"]);
	}
}
