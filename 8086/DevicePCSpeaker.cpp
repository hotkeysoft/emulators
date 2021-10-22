#include "DevicePCSpeaker.h"
#include <thread>
#include <assert.h>

#include <SDL.h>

namespace beeper
{
	DevicePCSpeaker::DevicePCSpeaker(WORD bufferSize) : 
		Logger("SPK"),
		m_bufferSize(bufferSize)
	{
		assert(emul::IsPowerOf2(bufferSize));

		m_bufSilence = new int8_t[m_bufferSize];
		m_bufPlaying = new int8_t[m_bufferSize];
		m_bufNext = new int8_t[m_bufferSize];

		Reset();
	}

	DevicePCSpeaker::~DevicePCSpeaker()
	{
		delete[] m_bufSilence;
		delete[] m_bufPlaying;
		delete[] m_bufNext;
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

		assert(length == spec.samples); // 8 bit mono, should match 1 to 1

		int8_t* dest = (int8_t*)stream;
		int8_t* source = This->IsFull() ? This->GetPlayingBuffer() : This->GetSilenceBuffer();

		memcpy(dest, source, length);
		if (This->IsFull())
		{
			This->ResetBuffer();
		}
	}

	void DevicePCSpeaker::InitSDLAudio()
	{
		SDL_AudioSpec want;
		want.freq = 44100;
		want.format = AUDIO_S8;
		want.channels = 1;
		want.samples = m_bufferSize;
		want.callback = &AudioCallback;
		want.userdata = this;

		m_audioDeviceID = SDL_OpenAudioDevice(0, 0, &want, &m_audioSpec, /*SDL_AUDIO_ALLOW_ANY_CHANGE*/0);

		for (WORD i = 0; i < m_bufferSize; ++i)
		{
			m_bufSilence[i] = m_audioSpec.silence;
			m_bufNext[i] = m_audioSpec.silence;
			m_bufPlaying[i] = m_audioSpec.silence;
		}

		SDL_PauseAudioDevice(m_audioDeviceID, false);
	}

	void DevicePCSpeaker::Tick(BYTE mixWith)
	{
		static int sample = 0;
		static int32_t avg = 0;

		// temp hack, avg 27 samples
		avg += (m_8255->IsSoundON() && m_8254->GetCounter(2).GetOutput()) ? 64 : 0;
		avg += mixWith;
		++sample;
		if (sample == 27)
		{
			avg /= 27; // Average
			avg -= 32; // Center around zero
			sample = 0;

			// Crude synchronization
			while (IsFull()) { std::this_thread::yield(); };

			AddSample(avg);
			avg = 0;
		}
	}

	uint16_t DevicePCSpeaker::PeriodToSamples(float period)
	{
		return (uint16_t)(period * m_audioSpec.freq / 1000000.0);
	}

}
