#pragma once

#include <Serializable.h>
#include <vector>

using emul::WORD;
using emul::BYTE;

namespace sound::vic
{
	class Voice : public Logger, public emul::Serializable
	{
	public:
		Voice(const char* label) : Logger(label) {}
		virtual ~Voice() {};

		Voice() = delete;
		Voice(const Voice&) = delete;
		Voice& operator=(const Voice&) = delete;
		Voice(Voice&&) = delete;
		Voice& operator=(Voice&&) = delete;

		virtual void Init() { Reset(); }
		virtual void Reset();
		void Tick();

		bool GetOutput() const { return m_out; }

		virtual void SetFrequency(BYTE value);

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		virtual void OnEndCount() { ToggleOutput(); }

		void ToggleOutput() { m_out = (m_enabled && !m_out); }
		void SetOutput(bool out) { m_out = (m_enabled && out); }

		BYTE m_n = 0xFF;
		BYTE m_counter = 0xFF;
		bool m_enabled = false;

	private:
		bool m_out = false;
	};

	class VoiceNoise : public Voice
	{
	public:
		VoiceNoise(const char* label);

		VoiceNoise() = delete;
		VoiceNoise(const VoiceNoise&) = delete;
		VoiceNoise& operator=(const VoiceNoise&) = delete;
		VoiceNoise(VoiceNoise&&) = delete;
		VoiceNoise& operator=(VoiceNoise&&) = delete;

		virtual void Reset() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		virtual void OnEndCount() override { Shift(); }

		void Shift();

		BYTE m_outShiftRegister = 0; // 8 bit output shift register
		WORD m_lfShiftRegister = 0; // 16 bit linear feedback shift register
	};

	class SoundVIC : public Logger, public emul::Serializable
	{
	public:
		SoundVIC();
		~SoundVIC();

		SoundVIC(const SoundVIC&) = delete;
		SoundVIC& operator=(const SoundVIC&) = delete;
		SoundVIC(SoundVIC&&) = delete;
		SoundVIC& operator=(SoundVIC&&) = delete;

		virtual void EnableLog(SEVERITY minSev = LOG_INFO);

		void Init();
		void Reset();

		void Tick();

		void SetVolume(BYTE vol) { m_volume = vol & 0x0F; }
		Voice& GetVoice(BYTE voice) { return *m_voices[voice]; }
		WORD GetOutput();

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		unsigned int m_counter = 0;
		BYTE m_volume = 0;
		Voice* m_voices[4];
	};
}
