#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Logger.h"

#include <vector>

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace sn76489
{
	static size_t s_clockSpeed = 1000000;

	static const BYTE s_volumeTable[16] = { 255, 203, 161, 128, 102, 81, 64, 51, 41, 32, 25, 20, 16, 13, 10, 0 };

	class Voice : public Logger
	{
	public:
		Voice(const char* label) : Logger(label) {}
		virtual ~Voice() {};

		Voice() = delete;
		Voice(const Voice&) = delete;
		Voice& operator=(const Voice&) = delete;
		Voice(Voice&&) = delete;
		Voice& operator=(Voice&&) = delete;

		virtual void Init() {}
		virtual void Tick() = 0;

		bool GetRawOutput() const { return m_out; }
		BYTE GetOutput() const { return m_out ? s_volumeTable[m_attenuation] : 0; }

		virtual void SetAttenuation(BYTE value);
		virtual void SetData(BYTE value, bool highLow) {}

	protected:
		void ToggleOutput() { m_out = !m_out; }
		void SetOutput(bool out) { m_out = out; }

		WORD m_n = 0b1111111111;
		WORD m_counter = 0b1111111111;

	private:
		BYTE m_attenuation = 0xF;
		bool m_out = false;
	};

	class VoiceSquare : public Voice
	{
	public:
		VoiceSquare(const char* label);

		VoiceSquare() = delete;
		VoiceSquare(const VoiceSquare&) = delete;
		VoiceSquare& operator=(const VoiceSquare&) = delete;
		VoiceSquare(VoiceSquare&&) = delete;
		VoiceSquare& operator=(VoiceSquare&&) = delete;

		virtual void Tick() override;

		virtual void SetData(BYTE value, bool highLow) override;
	};

	class VoiceNoise : public Voice
	{
	public:
		VoiceNoise(const char* label, Voice* trigger);

		VoiceNoise() = delete;
		VoiceNoise(const VoiceNoise&) = delete;
		VoiceNoise& operator=(const VoiceNoise&) = delete;
		VoiceNoise(VoiceNoise&&) = delete;
		VoiceNoise& operator=(VoiceNoise&&) = delete;

		virtual void Tick() override;

		virtual void SetData(BYTE value, bool) override;

	protected:
		Voice* m_trigger = nullptr;
		bool m_lastTrigger = false;

		bool m_whiteNoise = false;
		bool m_internalOutput = false;

		void Shift();
		void ResetShiftRegister() { m_shiftRegister = (1 << (m_shiftRegisterLen - 1)); }
 
		const BYTE m_shiftRegisterLen = 15;
		WORD m_shiftRegister;
		WORD m_noisePattern = 0b000000000010001;
		
	};

	class DeviceSN76489 : public PortConnector
	{
	public:
		DeviceSN76489(WORD baseAddress, size_t clockSpeedHz = 1000000);
		~DeviceSN76489();

		DeviceSN76489() = delete;
		DeviceSN76489(const DeviceSN76489&) = delete;
		DeviceSN76489& operator=(const DeviceSN76489&) = delete;
		DeviceSN76489(DeviceSN76489&&) = delete;
		DeviceSN76489& operator=(DeviceSN76489&&) = delete;

		virtual void EnableLog(SEVERITY minSev = LOG_INFO);

		void Init();
		void Reset();

		void Tick();

		WORD GetOutput();

		bool IsReady() const { return m_ready == 0; }

	protected:
		const BYTE m_tickDivider = 16;

		size_t m_ready = 0;
		void WriteData(BYTE value);

		const WORD m_baseAddress;

		Voice* m_currDest = nullptr;
		bool m_currFunc = false; // true = Attenuation, false = hi freq / noise mode
		Voice* m_voices[4];
	};
}
