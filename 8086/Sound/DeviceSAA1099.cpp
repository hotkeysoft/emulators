#include "stdafx.h"

#include "DeviceSAA1099.h"

using emul::GetBit;

namespace saa1099
{
	void Voice::Serialize(json& to)
	{
		to["n"] = m_n;
		to["counter"] = m_counter;
		to["out"] = m_out;
	}

	void Voice::Deserialize(const json& from)
	{
		m_n = from["n"];
		m_counter = from["counter"];
		m_out = from["out"];
	}

	// =================================

	VoiceSquare::VoiceSquare(std::string label) : Voice(label)
	{
		m_n = 1048576 * (1 << m_octave) / (511 - m_frequency);
	}

	void VoiceSquare::Init(VoiceNoise& noise)
	{
		m_noise = &noise;
	}

	void VoiceSquare::SetAmplitude(BYTE value)
	{
		m_amplitudeLeft = (value >> 0) & 15;
		m_amplitudeRight = (value >> 4) & 15;
		LogPrintf(LOG_DEBUG, "SetAmplitude, l[%02d] r[%02d]", m_amplitudeLeft, m_amplitudeRight);
	}
	void VoiceSquare::SetFrequency(BYTE value)
	{
		m_frequency = value;
		m_recomputeN = true;
		LogPrintf(LOG_DEBUG, "SetFrequency, f[%03d]", value);
	}
	void VoiceSquare::SetFrequencyEnable(bool enable)
	{
		m_frequencyEnable = enable;
		LogPrintf(LOG_DEBUG, "SetFrequencyEnable, enable=%d", enable);
	}
	void VoiceSquare::SetOctave(BYTE value)
	{
		m_octave = value;
		m_recomputeN = true;
		LogPrintf(LOG_DEBUG, "SetOctave, o[%d]", m_octave);
	}
	void VoiceSquare::SetNoiseEnable(bool enable)
	{
		m_noiseEnable = enable;
		LogPrintf(LOG_DEBUG, "SetNoiseEnable, enable=%d", enable);
	}

	void VoiceSquare::Tick()
	{
		m_counter += m_n;
		if (GetBit(m_counter, 23))
		{
			ToggleOutput();
			m_counter = 0;

			if (m_recomputeN)
			{
				m_n = 1048576 * (1 << m_octave) / (511 - m_frequency);
				m_recomputeN = false;
			}
		}
	}

	// =================================

	VoiceNoise::VoiceNoise(std::string label) :
		Voice(label)
	{
		ResetShiftRegister();
		SetOutput(false);
	}

	void VoiceNoise::Init(Voice& trigger)
	{
		m_trigger = &trigger;
		SetFrequency(2);
		m_counter = m_n;
	}

	void VoiceNoise::Tick()
	{	
		// Voice modulated
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
				//LogPrintf(LOG_DEBUG, "[%zu] Tick", emul::g_ticks);
				Shift();
			}
		}
	}

	void VoiceNoise::SetFrequency(BYTE value)
	{
		switch (value)
		{
		case 0: // 31.25kHz
			LogPrintf(LOG_DEBUG, "Noise Frequency: 31.25kHz");
			m_n = 8; // TODO
			break;
		case 1: // 15.6kHz
			LogPrintf(LOG_DEBUG, "Noise Frequency: 15.6kHz");
			m_n = 16; // TODO
			break;
		case 2: // 7.8kHz
			LogPrintf(LOG_DEBUG, "Noise Frequency: 7.8kHz");
			m_n = 32; // TODO
			break;
		case 3: // Freq generator [0|3]
			LogPrintf(LOG_WARNING, "Noise Frequency from tone generator: Not implemented");
			m_n = 0; // TODO
			break;
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
		bool input = parity(m_shiftRegister & m_noisePattern);

		m_shiftRegister = (m_shiftRegister >> 1);
		m_shiftRegister |= (input << (m_shiftRegisterLen - 1));

		SetOutput(m_shiftRegister & 1);
	}

	void VoiceNoise::Serialize(json& to)
	{
		Voice::Serialize(to);

		to["lastTrigger"] = m_lastTrigger;
		to["internalOutput"] = m_internalOutput;
		to["shiftRegister"] = m_shiftRegister;
	}
	void VoiceNoise::Deserialize(const json& from)
	{
		Voice::Deserialize(from);

		m_lastTrigger = from["lastTrigger"];
		m_internalOutput = from["internalOutput"];
		m_shiftRegister = from["shiftRegister"];
	}

	// =================================

	DeviceSAA1099::DeviceSAA1099(WORD baseAddress, const char* id) :
		Logger(id),
		m_baseAddress(baseAddress),
		m_voices {
			VoiceSquare(std::string(id) + ".0"),
			VoiceSquare(std::string(id) + ".1"),
			VoiceSquare(std::string(id) + ".2"),
			VoiceSquare(std::string(id) + ".3"),
			VoiceSquare(std::string(id) + ".4"),
			VoiceSquare(std::string(id) + ".5")
		},
		m_noise{
			VoiceNoise(std::string(id) + ".noise.0"),
			VoiceNoise(std::string(id) + ".noise.1"),
		}
	{
		Reset();
	}

	DeviceSAA1099::~DeviceSAA1099()
	{
	}

	void DeviceSAA1099::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		for (auto& voice : m_voices)
		{
			voice.EnableLog(minSev);
		}
		for (auto& noise : m_noise)
		{
			noise.EnableLog(minSev);
		}
	}

	void DeviceSAA1099::Reset()
	{
	}

	void DeviceSAA1099::Init()
	{
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&DeviceSAA1099::WriteData));
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&DeviceSAA1099::WriteAddress));

		const int ratio = VOICE_COUNT / NOISE_COUNT;
		
		// The two noise channels are shared among the 6 voices
		for (int i = 0; i < VOICE_COUNT; ++i)
		{
			m_voices[i].Init(m_noise[i / ratio]);
		}

		// Noise channel [0,1] can be triggered by voices [0,3] respectively.
		for (int i = 0; i < NOISE_COUNT; ++i)
		{
			m_noise[i].Init(m_voices[i * ratio]);
		}
	}

	void DeviceSAA1099::WriteData(BYTE value)
	{
		LogPrintf(LOG_TRACE, "WriteData, value=%02Xh", value);

		size_t voiceIndex;

		switch (m_currAddress)
		{
		case Address::REG_AMPLITUDE_0:
		case Address::REG_AMPLITUDE_1:
		case Address::REG_AMPLITUDE_2:
		case Address::REG_AMPLITUDE_3:
		case Address::REG_AMPLITUDE_4:
		case Address::REG_AMPLITUDE_5:
			voiceIndex = (size_t)m_currAddress - (size_t)Address::REG_AMPLITUDE_0;
			m_voices[voiceIndex].SetAmplitude(value);
			break;
		case Address::REG_FREQENCY_0:
		case Address::REG_FREQENCY_1:
		case Address::REG_FREQENCY_2:
		case Address::REG_FREQENCY_3:
		case Address::REG_FREQENCY_4:
		case Address::REG_FREQENCY_5:
			voiceIndex = (size_t)m_currAddress - (size_t)Address::REG_FREQENCY_0;
			m_voices[voiceIndex].SetFrequency(value);
			break;

		case Address::REG_OCTAVE_1_0:
		case Address::REG_OCTAVE_3_2:
		case Address::REG_OCTAVE_5_4:
			voiceIndex = ((size_t)m_currAddress - (size_t)Address::REG_OCTAVE_1_0) * 2;
			m_voices[voiceIndex + 1].SetOctave((value >> 4) & 7);
			m_voices[voiceIndex + 0].SetOctave((value >> 0) & 7);
			break;

		case Address::REG_FREQUENCY_EN:
			m_voices[0].SetFrequencyEnable(GetBit(value, 0));
			m_voices[1].SetFrequencyEnable(GetBit(value, 1));
			m_voices[2].SetFrequencyEnable(GetBit(value, 2));
			m_voices[3].SetFrequencyEnable(GetBit(value, 3));
			m_voices[4].SetFrequencyEnable(GetBit(value, 4));
			m_voices[5].SetFrequencyEnable(GetBit(value, 5));
			break;

		case Address::REG_NOISE_EN:
			m_voices[0].SetNoiseEnable(GetBit(value, 0));
			m_voices[1].SetNoiseEnable(GetBit(value, 1));
			m_voices[2].SetNoiseEnable(GetBit(value, 2));
			m_voices[3].SetNoiseEnable(GetBit(value, 3));
			m_voices[4].SetNoiseEnable(GetBit(value, 4));
			m_voices[5].SetNoiseEnable(GetBit(value, 5));
			break;

		case Address::REG_NOISE_GEN_1_0:
			m_noise[0].SetFrequency((value >> 0) & 3);
			m_noise[1].SetFrequency((value >> 4) & 3);
			break;

		case Address::REG_ENVELOPE_0:
			LogPrintf(LOG_DEBUG, "Set Envelope Generator 0, value=%02Xh", value);
			if (GetBit(value, 7))
			{
				LogPrintf(LOG_WARNING, "Set Envelope Generator 0: Envelope control not implemented");
			}
			break;
		case Address::REG_ENVELOPE_1:
			LogPrintf(LOG_DEBUG, "Set Envelope Generator 1, value=%02Xh", value);
			if (GetBit(value, 7))
			{
				LogPrintf(LOG_WARNING, "Set Envelope Generator 1: Envelope control not implemented");
			}
			break;

		case Address::REG_MISC:
			LogPrintf(LOG_WARNING, "Set Frequency Reset / Sound Enable: Not implemented");
			break;
		}
	}

	void DeviceSAA1099::WriteAddress(BYTE value)
	{
		LogPrintf(LOG_TRACE, "WriteAddress, value=%02Xh", value);
		m_currAddress = (Address)(value & 31);
	}

	void DeviceSAA1099::Tick()
	{
		if (--m_cooldown != 0)
			return;
		m_cooldown = m_tickDivider;

		for (auto& voice : m_voices)
		{
			voice.Tick();
		}
		for (auto& noise : m_noise)
		{
			noise.Tick();
		}
	}

	OutputData DeviceSAA1099::GetOutput() const
	{
		OutputData out;

		for (auto& voice : m_voices)
		{
			out.Mix(voice.GetOutput());
		}
		return out;
	}

	void DeviceSAA1099::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;

		for (auto& voice : m_voices)
		{
			voice.Serialize(to[voice.GetId()]);
		}
	}

	void DeviceSAA1099::Deserialize(const json& from)
	{
		WORD baseAddress = from["baseAddress"];
		if (baseAddress != m_baseAddress)
		{
			throw emul::SerializableException("DeviceSAA1099: Incompatible baseAddress");
		}

		for (auto& voice : m_voices)
		{
			voice.Deserialize(from[voice.GetId()]);
		}
	}
}
