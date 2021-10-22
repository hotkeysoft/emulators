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

		BYTE GetOutput() const { return m_out ? (15 - m_attenuation) : 0; }

		virtual void SetAttenuation(BYTE value);
		virtual void SetLowData(BYTE value) {}
		virtual void SetHighData(BYTE value) {}

	protected:
		void ToggleOutput() { m_out = !m_out; }
		void SetOutput(bool out) { m_out = out; }

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

		virtual void SetLowData(BYTE value) override;
		virtual void SetHighData(BYTE value) override;

	protected:
		const BYTE m_tickDivider = 16;

		WORD m_n = 0b1111111111;
		WORD m_counter = 0b1111111111;
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

		virtual void SetLowData(BYTE value) override;

	protected:
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

		virtual void EnableLog(bool enable, SEVERITY minSev = LOG_INFO);

		void Init();
		void Reset();

		void Tick();

		BYTE GetOutput();

	protected:
		void WriteData(BYTE value);

		const WORD m_baseAddress;

		Voice* m_currDest = nullptr;
		Voice* m_voices[4];
	};
}
