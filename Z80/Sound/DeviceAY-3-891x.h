#pragma once

#include <Serializable.h>
#include <CPU/PortConnector.h>

using emul::PortConnector;

using emul::BYTE;
using emul::WORD;

namespace sound::ay3
{
	class EventHandler
	{
	public:
		virtual BYTE OnReadPortA() { return 0xFF; }
		virtual BYTE OnReadPortB() { return 0xFF; }
	};

	// Log
	static const BYTE s_volumeTable[16] = { 0, 10, 13, 16, 20, 25, 32, 41, 51, 64, 81, 102, 128, 161, 203, 255 };

	// Linear
	//static const BYTE s_volumeTable[16] = { 0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255 };

	enum class Command
	{
		INACTIVE = 0b00,
		READ_DATA = 0b01,
		WRITE_DATA = 0b10,
		LATCH_ADDRESS = 0b11
	};

	enum class Address
	{
		// Tone Generator Control (R0-R5)
		REG_FREQUENCY_FINE_A,   // R0
		REG_FREQUENCY_COARSE_A, // R1
		REG_FREQUENCY_FINE_B,   // R2
		REG_FREQUENCY_COARSE_B, // R3
		REG_FREQUENCY_FINE_C,   // R4
		REG_FREQUENCY_COARSE_C, // R5

		// Noise Generator Control (R6)
		REG_FREQUENCY_NOISE,    // R6

		// Mixer Control / IO Enable
		REG_MIXER_IO,           // R7

		// R8-R9 skipped because registers
		// use octal notation

		// Amplitude Control (R10-R12)
		REG_AMPLITUDE_A,		// R10
		REG_AMPLITUDE_B,		// R11
		REG_AMPLITUDE_C,		// R12

		// Envelope Generator Control (R13-R15)
		REG_ENVELOPE_FINE,		// R13
		REG_ENVELOPE_COARSE,	// R14

		REG_ENVELOPE_SHAPE,		// R15

		REG_IO_PORT_A,			// R16
		REG_IO_PORT_B,			// R17

		_REG_MAX,

		REG_INVALID = 0xFF
	};

	struct OutputData
	{
		OutputData() {};
		OutputData(WORD a, WORD b, WORD c) : A(a), B(b), C(c) {}

		WORD A = 0;
		WORD B = 0;
		WORD C = 0;
	};

	class Voice : public Logger, public emul::Serializable
	{
	public:
		Voice(std::string label) : m_id(label), Logger(label.c_str()) {}
		virtual ~Voice() {};

		Voice() = delete;
		Voice(const Voice&) = delete;
		Voice& operator=(const Voice&) = delete;
		Voice(Voice&&) = delete;
		Voice& operator=(Voice&&) = delete;

		std::string GetId() const { return m_id; }

		virtual void Reset();
		virtual void Tick() = 0;

		bool GetRawOutput() const { return m_out; }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		void ToggleOutput() { m_out = !m_out; }
		void SetOutput(bool out) { m_out = out; }

		void IncCounter5() { ++m_counter; m_counter &= 31; }
		void IncCounter12() { ++m_counter; m_counter &= 4095; }

		emul::WORD m_n = 0;
		emul::WORD m_counter = 0;
		WORD m_frequency = 0;
		bool m_recomputeN = false;

	protected:
		std::string m_id;
		bool m_out = false;
	};

	class VoiceEnvelope : public Voice
	{
	public:
		VoiceEnvelope(std::string label);

		VoiceEnvelope() = delete;
		VoiceEnvelope(const VoiceEnvelope&) = delete;
		VoiceEnvelope& operator=(const VoiceEnvelope&) = delete;
		VoiceEnvelope(VoiceEnvelope&&) = delete;
		VoiceEnvelope& operator=(VoiceEnvelope&&) = delete;

		virtual void Reset() override;
		virtual void Tick() override;

		void SetFrequencyFine(BYTE value);
		void SetFrequencyCoarse(BYTE value);

		void SetEnvelopeShape(BYTE value);

		BYTE GetAmplitude() const { return m_amplitude; }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		BYTE m_amplitude = 0; // [0..15] Current amplitude

		void IncE() { ++m_e; m_e &= 15; }
		BYTE m_e = 0; // [0..15], position in cycle

		// Envelope shape
		bool m_continue = false;
		bool m_attack = false;
		bool m_alternate = false;
		bool m_hold = false;

		bool m_stopped = false;
		bool m_down = false;
	};

	class VoiceNoise : public Voice
	{
	public:
		VoiceNoise(std::string label);

		VoiceNoise() = delete;
		VoiceNoise(const VoiceNoise&) = delete;
		VoiceNoise& operator=(const VoiceNoise&) = delete;
		VoiceNoise(VoiceNoise&&) = delete;
		VoiceNoise& operator=(VoiceNoise&&) = delete;

		virtual void Reset() override;
		virtual void Tick() override;

		void SetFrequency(BYTE value);

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		bool m_internalOutput = false;

		void Shift();
		void ResetShiftRegister() { m_shiftRegister = (1 << (m_shiftRegisterLen - 1)); }

		const BYTE m_shiftRegisterLen = 17;
		const DWORD m_noisePattern = 0b00000000000001001;
		DWORD m_shiftRegister;
	};

	class VoiceSquare : public Voice
	{
	public:
		VoiceSquare(std::string label);

		VoiceSquare() = delete;
		VoiceSquare(const VoiceSquare&) = delete;
		VoiceSquare& operator=(const VoiceSquare&) = delete;
		VoiceSquare(VoiceSquare&&) = delete;
		VoiceSquare& operator=(VoiceSquare&&) = delete;

		void Init(VoiceNoise& noise, VoiceEnvelope& envelope);
		virtual void Reset() override;
		virtual void Tick() override;

		WORD GetOutput() const
		{
			const BYTE amplitude = (m_amplitudeMode == AmplitudeMode::FIXED) ? m_amplitude : m_envelope->GetAmplitude();
			const WORD dac = s_volumeTable[amplitude] * 32;

			return
				((m_toneEnable ? m_out : true) * dac) +
				((m_noiseEnable ? m_noise->GetRawOutput() : true) * dac);
		}

		void SetAmplitude(BYTE value);
		void SetFrequencyFine(BYTE value);
		void SetFrequencyCoarse(BYTE value);
		void SetNoiseEnable(bool enable);
		void SetToneEnable(bool enable);

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		enum class AmplitudeMode { FIXED = 0, ENVELOPE = 1 } m_amplitudeMode = AmplitudeMode::FIXED;
		BYTE m_amplitude = 0;

		bool m_toneEnable = false;
		bool m_noiseEnable = false;

		VoiceNoise* m_noise = nullptr;
		VoiceEnvelope* m_envelope = nullptr;
	};


	class DeviceAY_3_891x : public PortConnector, public emul::Serializable
	{
	public:
		DeviceAY_3_891x(const char* id = "ay3");
		~DeviceAY_3_891x();

		DeviceAY_3_891x(const DeviceAY_3_891x&) = delete;
		DeviceAY_3_891x& operator=(const DeviceAY_3_891x&) = delete;
		DeviceAY_3_891x(DeviceAY_3_891x&&) = delete;
		DeviceAY_3_891x& operator=(DeviceAY_3_891x&&) = delete;

		virtual void EnableLog(SEVERITY minSev = LOG_INFO);

		void Init();
		void Reset();

		void Tick();

		OutputData GetOutput() const
		{
			return OutputData(
				GetVoice(VoiceID::A).GetOutput(),
				GetVoice(VoiceID::B).GetOutput(),
				GetVoice(VoiceID::C).GetOutput());
		}

		enum class VoiceID
		{
			A,
			B,
			C,
			_MAX
		};

		VoiceSquare& GetVoice(VoiceID v) { assert(v < VoiceID::_MAX);  return m_voices[(int)v]; }
		const VoiceSquare& GetVoice(VoiceID v) const { assert(v < VoiceID::_MAX);  return m_voices[(int)v]; }

		void SetCommand(Command c);
		void WriteData(BYTE data);
		BYTE ReadData();

		void SetEventHandler(EventHandler* handler) { m_events = handler; }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		const BYTE m_tickDivider = 32; // TODO: Hardcoded for 1MHz clock
		BYTE m_cooldown = m_tickDivider;

		Address m_currAddress = Address::REG_INVALID;

		std::array<BYTE, (int)Address::_REG_MAX> m_registers;

		std::array<VoiceSquare, (int)VoiceID::_MAX> m_voices;
		VoiceNoise m_noise;
		VoiceEnvelope m_envelope;

		EventHandler* m_events = nullptr;

		enum class PortMode { INPUT, OUTPUT };
		PortMode m_portModeA = PortMode::INPUT;
		PortMode m_portModeB = PortMode::INPUT;

		void SetRegisterAddress();
		void SetRegisterData();
		void GetRegisterData();
		Command m_currCommand = Command::INACTIVE;
		mutable BYTE m_data = 0;
	};
}
