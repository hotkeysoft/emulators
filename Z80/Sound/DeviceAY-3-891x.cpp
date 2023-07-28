#include "stdafx.h"

#include "DeviceAY-3-891x.h"

using emul::GetBit;

namespace sound::ay3
{
	const char* GetCommandString(Command command)
	{
		switch (command)
		{
		case Command::WRITE_DATA: return "Write Data";
		case Command::LATCH_ADDRESS: return "Latch Address";
		case Command::READ_DATA: return "Read Data";
		case Command::INACTIVE: return "Inactive";
		default: throw std::exception("not possible");
		}
	}

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
	void VoiceSquare::SetFrequencyFine(BYTE value)
	{
		emul::SetLByte(m_frequency, value);
		m_recomputeN = true;
		LogPrintf(LOG_DEBUG, "SetFrequencyFine  [%02x]", value);
	}
	void VoiceSquare::SetFrequencyCoarse(BYTE value)
	{
		emul::SetHByte(m_frequency, value & 15);
		m_recomputeN = true;
		LogPrintf(LOG_DEBUG, "SetFrequencyCoarse [%02x]", value);
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

	void VoiceNoise::Init()
	{
		SetFrequency(2);
		m_counter = m_n;
	}

	void VoiceNoise::Tick()
	{
		if (--m_counter == 0)
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

	DeviceAY_3_891x::DeviceAY_3_891x(const char* id) :
		Logger(id),
		m_voices {
			VoiceSquare(std::string(id) + ".0"),
			VoiceSquare(std::string(id) + ".1"),
			VoiceSquare(std::string(id) + ".2")
		},
		m_noise {
			VoiceNoise(std::string(id) + ".noise")
		}
	{
		Reset();
	}

	DeviceAY_3_891x::~DeviceAY_3_891x()
	{
	}

	void DeviceAY_3_891x::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		for (auto& voice : m_voices)
		{
			voice.EnableLog(minSev);
		}
		m_noise.EnableLog(minSev);
	}

	void DeviceAY_3_891x::Reset()
	{
	}

	void DeviceAY_3_891x::Init()
	{
		// TODO: No port connection for now.
		//   only used in the Amstrad CPC, which calls Read/Write functions directly

		m_noise.Init();
		for (int i = 0; i < (int)VoiceID::_MAX; ++i)
		{
			m_voices[i].Init(m_noise);
		}
	}

	void DeviceAY_3_891x::SetCommand(Command command)
	{
		// address latch or data write is executed
		// when control pins go to inactive
		if (command == Command::INACTIVE)
		{
			// Run "last" command
			switch (m_currCommand)
			{
			case Command::WRITE_DATA:
				SetRegisterData();
				break;
			case Command::LATCH_ADDRESS:
				SetRegisterAddress();
				break;
			case Command::READ_DATA:
				GetRegisterData();
				break;
			case Command::INACTIVE:
				// Nothing to do
				break;
			default:
				throw std::exception("not possible");
			}
		}
		else
		{
			LogPrintf(LOG_INFO, "SetCommand: %d", GetCommandString(command));
		}

		m_currCommand = command;
	}

	void DeviceAY_3_891x::SetRegisterAddress()
	{
		m_currAddress = (Address)(m_data & 15);
		LogPrintf(LOG_INFO, "SetRegisterAddress: %02o", m_currAddress);
	}

	void DeviceAY_3_891x::GetRegisterData()
	{
		assert(m_currAddress < Address::_REG_MAX);
		// Put current register value in data latch
		m_data = m_registers[(int)m_currAddress];
	}

	void DeviceAY_3_891x::SetRegisterData()
	{
		LogPrintf(LOG_TRACE, "SetRegisterData, value=%02Xh", m_data);

		switch (m_currAddress)
		{
		case Address::REG_FREQUENCY_FINE_A:
			GetVoice(VoiceID::A).SetFrequencyFine(m_data);
			break;
		case Address::REG_FREQUENCY_COARSE_A:
			GetVoice(VoiceID::A).SetFrequencyCoarse(m_data);
			break;

		case Address::REG_FREQUENCY_FINE_B:
			GetVoice(VoiceID::B).SetFrequencyFine(m_data);
			break;
		case Address::REG_FREQUENCY_COARSE_B:
			GetVoice(VoiceID::B).SetFrequencyCoarse(m_data);
			break;

		case Address::REG_FREQUENCY_FINE_C:
			GetVoice(VoiceID::C).SetFrequencyFine(m_data);
			break;
		case Address::REG_FREQUENCY_COARSE_C:
			GetVoice(VoiceID::C).SetFrequencyCoarse(m_data);
			break;

		case Address::REG_FREQUENCY_NOISE:
		case Address::REG_MIXER_IO:
		case Address::REG_AMPLITUDE_A:
		case Address::REG_AMPLITUDE_B:
		case Address::REG_AMPLITUDE_C:
		case Address::REG_ENVELOPE_FINE:
		case Address::REG_ENVELOPE_COARSE:
		case Address::REG_ENVELOPE_SHAPE:
		case Address::REG_IO_PORT_A:
		case Address::REG_IO_PORT_B:
			LogPrintf(LOG_WARNING, "Not implemented");
			break;

		default:
			throw std::exception("not possible");
		}

		m_registers[(int)m_currAddress] = m_data;
	}

	void DeviceAY_3_891x::Tick()
	{
		if (--m_cooldown != 0)
			return;
		m_cooldown = m_tickDivider;

		for (auto& voice : m_voices)
		{
			voice.Tick();
		}
		m_noise.Tick();
	}

	OutputData DeviceAY_3_891x::GetOutput() const
	{
		OutputData out;

		for (auto& voice : m_voices)
		{
			out.Mix(voice.GetOutput());
		}
		return out;
	}

	void DeviceAY_3_891x::Serialize(json& to)
	{
		to["registers"] = m_registers;

		for (auto& voice : m_voices)
		{
			voice.Serialize(to[voice.GetId()]);
		}
		m_noise.Serialize(to[m_noise.GetId()]);
	}

	void DeviceAY_3_891x::Deserialize(const json& from)
	{
		m_registers = from["registers"];

		for (auto& voice : m_voices)
		{
			voice.Deserialize(from[voice.GetId()]);
		}
		m_noise.Deserialize(from[m_noise.GetId()]);
	}
}
