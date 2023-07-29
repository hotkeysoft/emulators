#include "stdafx.h"

#include "DeviceAY-3-891x.h"

using emul::GetBit;

namespace sound::ay3
{
	static EventHandler s_defaultHandler;

	const char* GetCommandString(Command command)
	{
		switch (command)
		{
		case Command::WRITE_DATA: return "Write Data";
		case Command::LATCH_ADDRESS: return "Latch Address";
		case Command::READ_DATA: return "Read Data";
		case Command::INACTIVE: return "Inactive";
		default: return "[invalid]";
		}
	}

	const char* GetAddressString(Address address)
	{
		switch (address)
		{
		case Address::REG_FREQUENCY_FINE_A: return "Frequency.A.Fine";
		case Address::REG_FREQUENCY_COARSE_A: return "Frequency.A.Coarse";
		case Address::REG_FREQUENCY_FINE_B: return "Frequency.B.Fine";
		case Address::REG_FREQUENCY_COARSE_B: return "Frequency.C.Coarse";
		case Address::REG_FREQUENCY_FINE_C: return "Frequency.C.Fine";
		case Address::REG_FREQUENCY_COARSE_C: return "Frequency.C.Coarse";
		case Address::REG_FREQUENCY_NOISE: return "Frequency.Noise";
		case Address::REG_MIXER_IO: return "Mixer/IO";
		case Address::REG_AMPLITUDE_A: return "Amplitude.A";
		case Address::REG_AMPLITUDE_B: return "Amplitude.B";
		case Address::REG_AMPLITUDE_C: return "Amplitude.C";
		case Address::REG_ENVELOPE_FINE: return "Envelope.Fine";
		case Address::REG_ENVELOPE_COARSE: return "Envelope.Coarse";
		case Address::REG_ENVELOPE_SHAPE: return "Envelope.Shape";
		case Address::REG_IO_PORT_A: return "IO.Port.A";
		case Address::REG_IO_PORT_B: return "IO.Port.B";
		default: return "[invalid]";
		}
	}

	void Voice::Serialize(json& to)
	{
		to["n"] = m_n;
		to["counter"] = m_counter;
		to["out"] = m_out;
		to["frequency"] = m_frequency;
		to["recomputeN"] = m_recomputeN;
	}

	void Voice::Deserialize(const json& from)
	{
		m_n = from["n"];
		m_counter = from["counter"];
		m_out = from["out"];
		m_frequency = from["frequency"];
		m_recomputeN = from["recomputeN"];
	}

	// =================================

	VoiceSquare::VoiceSquare(std::string label) : Voice(label)
	{
		Reset();
	}

	void VoiceSquare::Init(VoiceNoise& noise, VoiceEnvelope& envelope)
	{
		m_noise = &noise;
		m_envelope = &envelope;
	}

	void VoiceSquare::Reset()
	{
		Voice::Reset();
		m_amplitude = 0;
		m_frequency = 1;
		m_recomputeN = false;
		m_toneEnable = false;
		m_noiseEnable = false;
	}

	void VoiceSquare::SetAmplitude(BYTE value)
	{
		m_amplitudeMode = (AmplitudeMode)GetBit(value, 4);
		m_amplitude = (value & 15);

		LogPrintf(LOG_DEBUG, "SetAmplitude, [%C][%02x]", (m_amplitudeMode == AmplitudeMode::FIXED ? 'F' : 'E'), m_amplitude);
	}
	void VoiceSquare::SetFrequencyFine(BYTE value)
	{
		emul::SetLByte(m_frequency, value);
		m_recomputeN = true;
		LogPrintf(LOG_DEBUG, "SetFrequencyFine   [%02x]", value);
	}
	void VoiceSquare::SetFrequencyCoarse(BYTE value)
	{
		emul::SetHByte(m_frequency, value);
		m_recomputeN = true;
		LogPrintf(LOG_DEBUG, "SetFrequencyCoarse [%02x]", value);
	}

	void VoiceSquare::SetNoiseEnable(bool enable)
	{
		m_noiseEnable = enable;
		LogPrintf(LOG_DEBUG, "SetNoiseEnable, enable=%d", enable);
	}

	void VoiceSquare::SetToneEnable(bool enable)
	{
		m_toneEnable = enable;
		LogPrintf(LOG_DEBUG, "SetToneEnable, enable=%d", enable);
	}

	void VoiceSquare::Tick()
	{
		if (m_counter == m_n)
		{
			if (m_recomputeN)
			{
				m_n = m_frequency ? m_frequency : 1;
				m_recomputeN = false;
			}

			ToggleOutput();
			m_counter = 0;
		}
		else
		{
			IncCounter12();
		}
	}

	void VoiceSquare::Serialize(json& to)
	{
		Voice::Serialize(to);

		to["amplitudeMode"] = m_amplitudeMode;
		to["amplitude"] = m_amplitude;
		to["noiseEnable"] = m_noiseEnable;
		to["toneEnable"] = m_toneEnable;
	}
	void VoiceSquare::Deserialize(const json& from)
	{
		Voice::Deserialize(from);

		m_amplitudeMode = from["amplitudeMode"];
		m_amplitude = from["amplitude"];
		m_noiseEnable = from["noiseEnable"];
		m_toneEnable = from["toneEnable"];
	}

	// =================================

	VoiceEnvelope::VoiceEnvelope(std::string label) :
		Voice(label)
	{
		Reset();
	}

	void VoiceEnvelope::Reset()
	{
		Voice::Reset();
		m_amplitude = 0;

		m_continue = true;
		m_attack = true;
		m_alternate = false;
		m_hold = true;

		m_stopped = false;
		m_down = false;

		m_frequency = 1024;
		m_n = 1024;
	}

	void VoiceEnvelope::Tick()
	{
		if (m_counter == m_n)
		{
			m_counter = 0;

			if (m_recomputeN)
			{
				m_n = m_frequency ? m_frequency : 1;
				m_recomputeN = false;
				m_stopped = false;
			}
			else if (!m_stopped)
			{
				m_amplitude = GetDirection() ? (15-m_e) : m_e;

				IncE();

				if (m_e == 0) // End of a cycle
				{
					if (m_alternate)
					{
						m_down = !m_down;
					}

					if (!m_continue || m_hold)
					{
						m_stopped = true;
						if (!m_continue)
						{
							m_amplitude = 0;
						}
						else
						{
							//const bool dir = m_down ^ (!m_attack);
							m_amplitude = GetDirection() ? m_e : (15 - m_e);
						}
					}
				}
			}
		}
		else
		{
			++m_counter;
		}
	}

	void VoiceEnvelope::SetFrequencyFine(BYTE value)
	{
		emul::SetLByte(m_frequency, value);
		m_recomputeN = true;
		LogPrintf(LOG_DEBUG, "SetFrequencyFine   [%02x]", value);
	}
	void VoiceEnvelope::SetFrequencyCoarse(BYTE value)
	{
		emul::SetHByte(m_frequency, value);
		m_recomputeN = true;
		LogPrintf(LOG_DEBUG, "SetFrequencyCoarse [%02x]", value);
	}

	void VoiceEnvelope::Serialize(json& to)
	{
		Voice::Serialize(to);

		to["amplitude"] = m_amplitude;
		to["e"] = m_e;

		to["continue"] = m_continue;
		to["attack"] = m_attack;
		to["alternate"] = m_alternate;
		to["hold"] = m_hold;
		to["stopped"] = m_stopped;
	}
	void VoiceEnvelope::Deserialize(const json& from)
	{
		Voice::Deserialize(from);

		m_amplitude = from["amplitude"];
		m_e = from["e"];

		m_continue = from["continue"];
		m_attack = from["attack"];
		m_alternate = from["alternate"];
		m_hold = from["hold"];

		m_stopped = from["stopped"];
	}

	// =================================


	VoiceNoise::VoiceNoise(std::string label) :
		Voice(label)
	{
		Reset();
	}

	void VoiceNoise::Reset()
	{
		Voice::Reset();
		ResetShiftRegister();
		m_internalOutput = false;
	}

	void VoiceNoise::Tick()
	{
		if (m_counter == m_n)
		{
			if (m_recomputeN)
			{
				m_n = m_frequency ? m_frequency : 1;
				m_recomputeN = false;
			}

			m_internalOutput = !m_internalOutput;
			if (m_internalOutput)
			{
				//LogPrintf(LOG_DEBUG, "[%zu] Tick", emul::g_ticks);
				Shift();
			}
			m_counter = 0;
		}
		else
		{
			IncCounter5();
		}
	}

	void VoiceNoise::SetFrequency(BYTE value)
	{
		m_frequency = value & 31;
		m_recomputeN = true;
		LogPrintf(LOG_DEBUG, "SetFrequency       [%02x]", m_frequency);
	}

	static bool parity(DWORD val)
	{
		val ^= (val >> 15);
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

	DeviceAY_3_891x::DeviceAY_3_891x(const char* id) :
		Logger(id),
		m_events(&s_defaultHandler),
		m_voices {
			VoiceSquare(std::string(id) + ".0"),
			VoiceSquare(std::string(id) + ".1"),
			VoiceSquare(std::string(id) + ".2")
		},
		m_noise(std::string(id) + ".noise"),
		m_envelope(std::string(id) + ".env")
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
		m_envelope.EnableLog(minSev);
	}

	void DeviceAY_3_891x::Reset()
	{
		m_portModeA = PortMode::INPUT;
		m_portModeB = PortMode::INPUT;

		m_envelope.Reset();
		m_noise.Reset();
		for (auto& voice : m_voices)
		{
			voice.Reset();
		}
	}

	void DeviceAY_3_891x::Init()
	{
		// TODO: No port connection for now.
		//   only used in the Amstrad CPC, which calls Read/Write functions directly

		for (auto& voice: m_voices)
		{
			voice.Init(m_noise, m_envelope);
		}
	}

	void DeviceAY_3_891x::SetCommand(Command command)
	{
		LogPrintf(LOG_TRACE, "SetCommand: %s", GetCommandString(command));
		m_currCommand = command;

		switch (m_currCommand)
		{
		case Command::INACTIVE:
			break;
		case Command::READ_DATA:
			GetRegisterData();
			break;
		case Command::LATCH_ADDRESS:
			SetRegisterAddress();
			break;
		case Command::WRITE_DATA:
			SetRegisterData();
			break;
		default:
			throw std::exception("not possible");
		}

	}

	void DeviceAY_3_891x::WriteData(BYTE data)
	{
		m_data = data;
	}
	BYTE DeviceAY_3_891x::ReadData()
	{
		return (m_currCommand == Command::INACTIVE) ? 0xFF : m_data;

	}

	void DeviceAY_3_891x::SetRegisterAddress()
	{
		// Upper 4 address bits are used as 'chip select'
		// Therefore nothing is selected if high nibble is != 0
		// (you could order chips with a different mask, but let's assume 0 for now)
		m_currAddress =  (m_data & 0xF0) ? Address::REG_INVALID : (Address)(m_data & 15);
		LogPrintf(LOG_TRACE, "SetRegisterAddress: %02o", m_currAddress);
	}

	void DeviceAY_3_891x::GetRegisterData()
	{
		assert(m_currAddress < Address::_REG_MAX);

		LogPrintf(LOG_TRACE, "GetRegisterData(%s)", GetAddressString(m_currAddress));

		// IO Ports: Reading an 'output' port will return the value of the register
		//           ANDed with the input of the port
		if (m_currAddress == Address::REG_INVALID)
		{
			m_data = 0xFF;
		}
		else if (m_currAddress == Address::REG_IO_PORT_A)
		{
			BYTE initialValue = (m_portModeA == PortMode::OUTPUT) ? m_registers[(int)Address::REG_IO_PORT_A] : 255;
			m_data = initialValue & m_events->OnReadPortA();
		}
		else if (m_currAddress == Address::REG_IO_PORT_B)
		{
			BYTE initialValue = (m_portModeB == PortMode::OUTPUT) ? m_registers[(int)Address::REG_IO_PORT_B] : 255;
			m_data = initialValue & m_events->OnReadPortB();
		}
		else
		{
			m_data = m_registers[(int)m_currAddress];
		}
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
			m_data &= 0x0F;
			GetVoice(VoiceID::A).SetFrequencyCoarse(m_data);
			break;

		case Address::REG_FREQUENCY_FINE_B:
			GetVoice(VoiceID::B).SetFrequencyFine(m_data);
			break;
		case Address::REG_FREQUENCY_COARSE_B:
			m_data &= 0x0F;
			GetVoice(VoiceID::B).SetFrequencyCoarse(m_data);
			break;

		case Address::REG_FREQUENCY_FINE_C:
			GetVoice(VoiceID::C).SetFrequencyFine(m_data);
			break;
		case Address::REG_FREQUENCY_COARSE_C:
			m_data &= 0x0F;
			GetVoice(VoiceID::C).SetFrequencyCoarse(m_data);
			break;

		case Address::REG_FREQUENCY_NOISE:
			m_data &= 0x1F;
			m_noise.SetFrequency(m_data);
			break;

		case Address::REG_MIXER_IO:
			GetVoice(VoiceID::A).SetToneEnable(!GetBit(m_data, 0));
			GetVoice(VoiceID::B).SetToneEnable(!GetBit(m_data, 1));
			GetVoice(VoiceID::C).SetToneEnable(!GetBit(m_data, 2));
			GetVoice(VoiceID::A).SetNoiseEnable(!GetBit(m_data, 3));
			GetVoice(VoiceID::B).SetNoiseEnable(!GetBit(m_data, 4));
			GetVoice(VoiceID::C).SetNoiseEnable(!GetBit(m_data, 5));
			m_portModeA = GetBit(m_data, 6) ? PortMode::OUTPUT : PortMode::INPUT;
			m_portModeB = GetBit(m_data, 7) ? PortMode::OUTPUT : PortMode::INPUT;
			break;

		case Address::REG_AMPLITUDE_A:
			m_data &= 0x1F;
			GetVoice(VoiceID::A).SetAmplitude(m_data);
			break;
		case Address::REG_AMPLITUDE_B:
			m_data &= 0x1F;
			GetVoice(VoiceID::B).SetAmplitude(m_data);
			break;
		case Address::REG_AMPLITUDE_C:
			m_data &= 0x1F;
			GetVoice(VoiceID::C).SetAmplitude(m_data);
			break;

		case Address::REG_ENVELOPE_FINE:
			m_envelope.SetFrequencyFine(m_data);
			break;
		case Address::REG_ENVELOPE_COARSE:
			m_envelope.SetFrequencyCoarse(m_data);
			break;
		case Address::REG_ENVELOPE_SHAPE:
			m_data &= 0x0F;
			LogPrintf(LOG_WARNING, "Not implemented: %s", GetAddressString(m_currAddress));
			break;

		case Address::REG_IO_PORT_A:
		case Address::REG_IO_PORT_B:
			// Nothing to do, store register value in m_registers below
			break;

		case Address::REG_INVALID:
			LogPrintf(LOG_DEBUG, "No address selected");
			return;

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
		m_envelope.Tick();
	}

	void DeviceAY_3_891x::Serialize(json& to)
	{
		to["registers"] = m_registers;

		for (auto& voice : m_voices)
		{
			voice.Serialize(to[voice.GetId()]);
		}
		m_noise.Serialize(to[m_noise.GetId()]);
		m_envelope.Serialize(to[m_envelope.GetId()]);
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
