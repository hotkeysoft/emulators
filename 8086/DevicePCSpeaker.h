#pragma once

#include "Common.h"
#include "Logger.h"

#include "Device8254.h"
#include "Device8255.h"

#include <SDL.h>

namespace beeper
{
	class DevicePCSpeaker : public Logger
	{
	public:
		DevicePCSpeaker();

		DevicePCSpeaker(const DevicePCSpeaker&) = delete;
		DevicePCSpeaker& operator=(const DevicePCSpeaker&) = delete;
		DevicePCSpeaker(DevicePCSpeaker&&) = delete;
		DevicePCSpeaker& operator=(DevicePCSpeaker&&) = delete;

		void Init(ppi::Device8255* ppi, pit::Device8254* pit);
		void Reset();

		void Tick();

		const SDL_AudioSpec& GetAudioSpec() const { return m_audioSpec; }
		uint16_t GetPeriod() { return m_soundPeriod; }
		bool GetNextSample();

	protected:
		uint16_t PeriodToSamples(float period);

		void InitSDLAudio();

		ppi::Device8255* m_8255 = nullptr;
		pit::Device8254* m_8254 = nullptr;

		uint16_t m_soundPeriod = 0;
		uint16_t m_onCount = 0;
		uint16_t m_offCount = 0;

		bool m_currSample = false;
		uint16_t m_currCount = 0;

		SDL_AudioSpec m_audioSpec;
		SDL_AudioDeviceID m_audioDeviceID;
	};
}
