#pragma once

#include <Serializable.h>
#include <CPU/PortConnector.h>

using emul::PortConnector;

using emul::BYTE;
using emul::WORD;

namespace sound::ay3
{
	// Log
	//static const BYTE s_volumeTable[16] = { 0, 10, 13, 16, 20, 25, 32, 41, 51, 64, 81, 102, 128, 161, 203, 255 };

	// Linear
	static const BYTE s_volumeTable[16] = { 0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255 };

	enum class Command
	{
		INACTIVE,
		READ_DATA,
		WRITE_DATA,
		LATCH_ADDRESS
	};

	struct OutputData
	{
		OutputData() {};
		OutputData(WORD l, WORD r) : left(l), right(r) {}
		void Mix(const OutputData& mix)
		{
			left += mix.left;
			right += mix.right;
		}
		void Mix(const OutputData& mix, int mul)
		{
			left += mix.left * mul;
			right += mix.right * mul;
		}

		WORD left = 0;
		WORD right = 0;
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

		virtual void Tick() = 0;

		bool GetRawOutput() const { return m_out; }

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		void ToggleOutput() { m_out = !m_out; }
		void SetOutput(bool out) { m_out = out; }

		emul::DWORD m_n = 0;
		emul::DWORD m_counter = 0;

	protected:
		std::string m_id;
		bool m_out = false;
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

		void Init();
		virtual void Tick() override;

		void SetFrequency(BYTE value);

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		bool m_lastTrigger = false;

		bool m_internalOutput = false;

		void Shift();
		void ResetShiftRegister() { m_shiftRegister = (1 << (m_shiftRegisterLen - 1)); }

		const BYTE m_shiftRegisterLen = 15;
		const WORD m_noisePattern = 0b000000000010001;
		WORD m_shiftRegister;
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

		void Init(VoiceNoise& noise);
		virtual void Tick() override;

		OutputData GetOutput() const
		{
			OutputData out;
			if (m_noiseEnable && m_noise->GetRawOutput())
			{
				out.Mix(OutputData(s_volumeTable[m_amplitudeLeft], s_volumeTable[m_amplitudeRight]));
			}
			if (m_frequencyEnable && m_out)
			{
				OutputData freqData(s_volumeTable[m_amplitudeLeft], s_volumeTable[m_amplitudeRight]);
				out.Mix(freqData, m_noiseEnable ? 4 : 1);
			}
			return out;
		}

		void SetAmplitude(BYTE value);
		void SetFrequencyFine(BYTE value);
		void SetFrequencyCoarse(BYTE value);
		void SetNoiseEnable(bool enable);

	protected:
		BYTE m_amplitudeLeft = 15;
		BYTE m_amplitudeRight = 15;
		WORD m_frequency = 0;
		bool m_recomputeN = false;
		BYTE m_octave = 0;
		bool m_frequencyEnable = false;
		bool m_noiseEnable = false;

		VoiceNoise* m_noise = nullptr;
	};

	class DeviceAY_3_891x : public PortConnector, public emul::Serializable
	{
	public:
		DeviceAY_3_891x(const char* id = "ay3");
		~DeviceAY_3_891x();

		DeviceAY_3_891x() = delete;
		DeviceAY_3_891x(const DeviceAY_3_891x&) = delete;
		DeviceAY_3_891x& operator=(const DeviceAY_3_891x&) = delete;
		DeviceAY_3_891x(DeviceAY_3_891x&&) = delete;
		DeviceAY_3_891x& operator=(DeviceAY_3_891x&&) = delete;

		virtual void EnableLog(SEVERITY minSev = LOG_INFO);

		void Init();
		void Reset();

		void Tick();

		OutputData GetOutput() const;

		enum class VoiceID
		{
			A,
			B,
			C,
			_MAX
		};

		VoiceSquare& GetVoice(VoiceID v) { assert(v < VoiceID::_MAX);  return m_voices[(int)v]; }

		void SetCommand(Command c);
		void WriteData(BYTE data) { m_data = data; }
		BYTE ReadData() const { return m_data; }

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		const BYTE m_tickDivider = 32;
		BYTE m_cooldown = m_tickDivider;

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
		} m_currAddress = Address::REG_INVALID;

		std::array<BYTE, (int)Address::_REG_MAX> m_registers;

		std::array<VoiceSquare, (int)VoiceID::_MAX> m_voices;
		VoiceNoise m_noise;

		void SetRegisterAddress();
		void SetRegisterData();
		void GetRegisterData();
		Command m_currCommand = Command::INACTIVE;
		BYTE m_data = 0;
	};
}
