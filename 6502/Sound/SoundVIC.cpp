#include "stdafx.h"

#include "SoundVIC.h"

using emul::GetBit;
using emul::SetBit;
using emul::GetLSB;
using emul::GetMSB;

namespace sound::vic
{
	void Voice::Reset()
	{
		m_n = 0xFF;
		m_counter = 0xFF;
	}

	void Voice::SetFrequency(BYTE value)
	{
		m_n = 127 - (value & 127);
		m_enabled = GetMSB(value);
	}

	void Voice::Tick()
	{
		if (--m_counter == 0)
		{
			m_counter = m_n;
			OnEndCount();
		}
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

	// =================================

	VoiceNoise::VoiceNoise(const char* label) : Voice(label)
	{
	}

	void VoiceNoise::Reset()
	{
		Voice::Reset();
		m_outShiftRegister = 0x0;
		m_lfShiftRegister = 0x0;
	}

	void VoiceNoise::Shift()
	{
		if (GetLSB(m_lfShiftRegister))
		{
			const bool bit = GetMSB(m_outShiftRegister);
			m_outShiftRegister <<= 1;
			SetBit(m_outShiftRegister, 0, !bit);
		}

		m_lfShiftRegister <<= 1;

		// http://sleepingelephant.com/ipw-web/bulletin/bb/viewtopic.php?f=11&t=8733&start=210#p104360
		const bool lfsr1 = (GetBit(m_lfShiftRegister, 3) ^ GetBit(m_lfShiftRegister, 12));
		const bool lfsr2 = (GetBit(m_lfShiftRegister, 14) ^ GetBit(m_lfShiftRegister, 15));
		const bool lfsr3 = lfsr1 ^ lfsr2;

		SetBit(m_lfShiftRegister, 0, !lfsr3);

		SetOutput(GetLSB(m_outShiftRegister));
	}

	void VoiceNoise::Serialize(json& to)
	{
		Voice::Serialize(to);

		to["outShiftRegister"] = m_outShiftRegister;
		to["lfShiftRegister"] = m_lfShiftRegister;
	}
	void VoiceNoise::Deserialize(const json& from)
	{
		Voice::Deserialize(from);

		m_outShiftRegister = from["outShiftRegister"];
		m_lfShiftRegister = from["lfShiftRegister"];
	}

	// =================================

	SoundVIC::SoundVIC() : Logger("soundVIC")
	{
		m_voices[0] = new Voice("vic_V0");
		m_voices[1] = new Voice("vic_V1");
		m_voices[2] = new Voice("vic_V2");
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

		if (GetBit(changed, 2)) m_voices[3]->Tick(); // CLK/8
	}

	WORD SoundVIC::GetOutput()
	{
		WORD out =
			(m_voices[0]->GetOutput() ? 1 : 0) +
			(m_voices[1]->GetOutput() ? 1 : 0) +
			(m_voices[2]->GetOutput() ? 1 : 0) +
			(m_voices[3]->GetOutput() ? 1 : 0);

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
