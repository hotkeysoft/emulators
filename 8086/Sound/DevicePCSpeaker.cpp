#include "DevicePCSpeaker.h"
#include <thread>
#include <assert.h>
#include <algorithm>

#include <SDL.h>

namespace beeper
{
	DevicePCSpeaker::DevicePCSpeaker(WORD bufferSize) : 
		Logger("SPK"),
		m_bufferSize(bufferSize)
	{
		assert(emul::IsPowerOf2(bufferSize));

		m_bufSilence = new int16_t[m_bufferSize];
		m_bufPlaying = new int16_t[m_bufferSize];
		m_bufStaging = new int16_t[m_bufferSize];

		Reset();
	}

	DevicePCSpeaker::~DevicePCSpeaker()
	{
		delete[] m_bufSilence;
		delete[] m_bufPlaying;
		delete[] m_bufStaging;
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

		assert(length == spec.samples * 2);

		const uint8_t* source = (const uint8_t*)This->GetPlayingBuffer();

		memset(stream, 0, length * sizeof(uint16_t));
		SDL_MixAudioFormat(stream, source, spec.format, length, This->GetMasterVolume());
		if (This->IsStagingFull())
		{
			This->ResetStaging();
		}
	}

	void DevicePCSpeaker::InitSDLAudio()
	{
		SDL_AudioSpec want;
		want.freq = 44100;
		want.format = AUDIO_S16;
		want.channels = 1;
		want.samples = m_bufferSize;
		want.callback = &AudioCallback;
		want.userdata = this;

		m_audioDeviceID = SDL_OpenAudioDevice(0, 0, &want, &m_audioSpec, /*SDL_AUDIO_ALLOW_ANY_CHANGE*/0);

		for (WORD i = 0; i < m_bufferSize; ++i)
		{
			m_bufSilence[i] = m_audioSpec.silence;
			m_bufStaging[i] = m_audioSpec.silence;
			m_bufPlaying[i] = m_audioSpec.silence;
		}

		SDL_PauseAudioDevice(m_audioDeviceID, false);
	}

	void DevicePCSpeaker::MoveToPlayingBuffer()
	{
		SDL_LockAudio();
		memcpy(m_bufPlaying, m_bufStaging, m_bufferSize * sizeof(uint16_t));
		SDL_UnlockAudio();
	}

	void DevicePCSpeaker::Tick(WORD mixWith)
	{
		static int sample = 0;
		static int32_t avg = 0;

		// temp hack, "average" 27 samples
		// possibly 4 incoming channels + this one, max possible value = 255*5 = 34425
		// could clip with 4 channels + pc speaker full blast but very unlikely
		BYTE speakerData = (m_8255->IsSoundON() && m_8254->GetCounter(2).GetOutput()) ? 128 : 0;
		if (m_outputFile) fputc(speakerData, m_outputFile);
		avg += speakerData;
		avg += mixWith;
		++sample;
		if (sample == 27)
		{
			if (IsStagingFull())
			{
				MoveToPlayingBuffer();
			}

			// Crude synchronization
			while (IsStagingFull()) { std::this_thread::yield(); };

			AddSample(m_muted ? 0 : avg);

			avg = 0;
			sample = 0;
		}
	}

	void DevicePCSpeaker::SetMasterVolume(int vol)
	{
		m_masterVolume = std::min(SDL_MIX_MAXVOLUME, std::max(0, vol));
		LogPrintf(LOG_INFO, "Set Master Volume [%d]", m_masterVolume);
	}

	uint16_t DevicePCSpeaker::PeriodToSamples(float period)
	{
		return (uint16_t)(period * m_audioSpec.freq / 1000000.0);
	}

	void DevicePCSpeaker::StreamToFile(bool stream, const char* outFile)
	{
		if (!stream && m_outputFile)
		{
			LogPrintf(LOG_INFO, "StreamToFile: Stop audio stream dump to file");
			fclose(m_outputFile);
			m_outputFile = nullptr;
			return;
		}
		else
		{
			if (!outFile)
			{
				outFile = "speaker.bin";
			}

			LogPrintf(LOG_INFO, "StreamToFile: Start audio stream dump to file [%s]", outFile);

			m_outputFile = fopen(outFile, "wb");
			if (!m_outputFile)
			{
				LogPrintf(LOG_ERROR, "StreamToFile: error opening file");
			}
		}
	}

}
