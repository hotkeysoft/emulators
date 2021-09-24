#include "DevicePCSpeaker.h"
#include <assert.h>

#include <SDL.h>

namespace beeper
{
	DevicePCSpeaker::DevicePCSpeaker() : Logger("SPK")
	{
		Reset();
	}

	void DevicePCSpeaker::Reset()
	{
	}

	void DevicePCSpeaker::Init(ppi::Device8255* ppi, pit::Device8254* pit)
	{
		assert(ppi);
		assert(pit);
		m_8254 = pit;
		m_8255 = ppi;


		if (SDL_WasInit(SDL_INIT_AUDIO) == 0)
		{
			if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
			{
				LogPrintf(LOG_ERROR, "Error initializing sound audio subsystem: %s", SDL_GetError());
			}
			else
			{
				InitSDLAudio();
			}
		}
	}

	void AudioCallback(void* userData, Uint8* stream, int length)
	{
		DevicePCSpeaker* This = (DevicePCSpeaker*)(userData);
		const SDL_AudioSpec& spec = This->GetAudioSpec();

		int8_t* buffer = (int8_t*)stream;

		for (int i = 0; i < length; ++i)
		{
			buffer[i] = This->GetNextSample();
		}
	}

	void DevicePCSpeaker::InitSDLAudio()
	{
		SDL_AudioSpec want;
		want.freq = 44100;
		want.format = AUDIO_S8;
		want.channels = 1;
		want.samples = 1024;
		want.callback = &AudioCallback;
		want.userdata = this;

		m_audioDeviceID = SDL_OpenAudioDevice(0, 0, &want, &m_audioSpec, /*SDL_AUDIO_ALLOW_ANY_CHANGE*/0);
		SDL_PauseAudioDevice(m_audioDeviceID, false);
	}

	void DevicePCSpeaker::Tick()
	{
		Uint16 period = PeriodToSamples(m_8254->GetCounter(2).GetPeriodMicro());
		if (period != m_soundPeriod)
		{
			m_soundPeriod = period;
			m_onCount = period / 2;
			m_offCount = period / 2;
			if (period & 1)
			{
				++m_onCount;
			}
			LogPrintf(LOG_INFO, "New period: %d samples (%d on / %d off)", period, m_onCount, m_offCount);
			
			m_currSample = true;
			m_currCount = m_onCount;
		}

	}

	uint8_t DevicePCSpeaker::GetNextSample()
	{
		if (!m_8255->IsSoundON())
		{
			return 0;
		}

		--m_currCount;

		if (m_currCount == 0)
		{
			m_currSample = !m_currSample;
			m_currCount = m_currSample ? m_offCount : m_onCount;
		}

		return m_currSample ? 127 : -128;
	}

	uint16_t DevicePCSpeaker::PeriodToSamples(float period)
	{
		return (uint16_t)(period * m_audioSpec.freq / 1000000.0);
	}

}
