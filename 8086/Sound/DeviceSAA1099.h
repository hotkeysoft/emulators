#pragma once

#include "../Serializable.h"
#include "../CPU/PortConnector.h"

using emul::PortConnector;

using emul::BYTE;
using emul::WORD;

namespace saa1099
{
	const int VOICE_COUNT = 6;
	const int NOISE_COUNT = 2;

	// Log
	//static const BYTE s_volumeTable[16] = { 0, 10, 13, 16, 20, 25, 32, 41, 51, 64, 81, 102, 128, 161, 203, 255 };

	// Linear
	static const BYTE s_volumeTable[16] = { 0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255 };

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

		void Init(Voice& trigger);
		virtual void Tick() override;

		void SetFrequency(BYTE value);

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		Voice* m_trigger = nullptr;
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
		void SetFrequency(BYTE value);
		void SetFrequencyEnable(bool enable);
		void SetOctave(BYTE value);
		void SetNoiseEnable(bool enable);

	protected:
		BYTE m_amplitudeLeft = 15;
		BYTE m_amplitudeRight = 15;
		BYTE m_frequency = 0;
		bool m_recomputeN = false;
		BYTE m_octave = 0;
		bool m_frequencyEnable = false;
		bool m_noiseEnable = false;

		VoiceNoise* m_noise = nullptr;
	};

	class DeviceSAA1099 : public PortConnector, public emul::Serializable
	{
	public:
		DeviceSAA1099(WORD baseAddress, const char* id = "saa1099");
		~DeviceSAA1099();

		DeviceSAA1099() = delete;
		DeviceSAA1099(const DeviceSAA1099&) = delete;
		DeviceSAA1099& operator=(const DeviceSAA1099&) = delete;
		DeviceSAA1099(DeviceSAA1099&&) = delete;
		DeviceSAA1099& operator=(DeviceSAA1099&&) = delete;

		virtual void EnableLog(SEVERITY minSev = LOG_INFO);

		void Init();
		void Reset();

		void Tick();

		OutputData GetOutput() const;

		VoiceSquare& GetVoice(BYTE index) { return m_voices[index]; }

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		const WORD m_baseAddress;

		const BYTE m_tickDivider = 32;
		BYTE m_cooldown = m_tickDivider;

		enum class Address
		{
			REG_AMPLITUDE_0 = 0x00,
			REG_AMPLITUDE_1,
			REG_AMPLITUDE_2,
			REG_AMPLITUDE_3,
			REG_AMPLITUDE_4,
			REG_AMPLITUDE_5,
			
			REG_FREQENCY_0 = 0x08,
			REG_FREQENCY_1,
			REG_FREQENCY_2,
			REG_FREQENCY_3,
			REG_FREQENCY_4,
			REG_FREQENCY_5,

			REG_OCTAVE_1_0 = 0x10,
			REG_OCTAVE_3_2,
			REG_OCTAVE_5_4,

			REG_FREQUENCY_EN = 0x14,

			REG_NOISE_EN = 0x15,
			REG_NOISE_GEN_1_0 = 0x16,

			REG_ENVELOPE_0 = 0x18,
			REG_ENVELOPE_1,

			REG_MISC = 0x1C,

			REG_INVALID = 0x1F
		} m_currAddress = Address::REG_INVALID;

		void WriteAddress(BYTE value);
		void WriteData(BYTE value);			

		std::array<VoiceSquare, VOICE_COUNT> m_voices;
		std::array<VoiceNoise, NOISE_COUNT> m_noise;
	};
}
