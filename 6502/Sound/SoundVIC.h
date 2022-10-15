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
		virtual void Tick() = 0;

		bool GetOutput() const { return m_out & m_enabled; }

		virtual void SetFrequency(BYTE value);

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		void ToggleOutput() { m_out = !m_out; }
		void SetOutput(bool out) { m_out = out; }

		BYTE m_n = 0xFF;
		BYTE m_counter = 0xFF;
		bool m_enabled = false;

	private:
		bool m_out = false;
	};

	class VoiceSquare : public Voice
	{
	public:
		VoiceSquare(const char* label);

		virtual void Tick() override;

		VoiceSquare() = delete;
		VoiceSquare(const VoiceSquare&) = delete;
		VoiceSquare& operator=(const VoiceSquare&) = delete;
		VoiceSquare(VoiceSquare&&) = delete;
		VoiceSquare& operator=(VoiceSquare&&) = delete;
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

		virtual void Tick() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		bool m_internalOutput = false;

		void Shift();
		void ResetShiftRegister() { m_shiftRegister = (1 << (m_shiftRegisterLen - 1)); }

		const BYTE m_shiftRegisterLen = 15;
		const WORD m_noisePattern = 0b000000000010001;
		WORD m_shiftRegister;
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
