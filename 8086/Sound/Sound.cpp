#include "stdafx.h"
#include "Sound.h"

#include <thread>
#include <algorithm>

namespace sound
{
	// Called from another thread
	void AudioCallback(void* userData, Uint8* stream, int length)
	{
		const SDL_AudioSpec& spec = SOUND().GetAudioSpec();

		assert(length == spec.samples * 2);

		const uint8_t* source = (const uint8_t*)SOUND().GetPlayingBuffer();

		memset(stream, 0, length * sizeof(uint16_t));
		SDL_MixAudioFormat(stream, source, spec.format, length, SOUND().GetMasterVolume());
		if (SOUND().IsStagingFull())
		{
			SOUND().ResetStaging();
		}
	}

	Sound& Sound::Get()
	{
		static Sound sound;
		return sound;
	}

	Sound::Sound() : Logger("sound")
	{
	}

	Sound::~Sound()
	{
		SDL_CloseAudioDevice(m_audioDeviceID);
		delete[] m_bufSilence;
		delete[] m_bufPlaying;
		delete[] m_bufStaging;
	}

	bool Sound::Init(WORD bufferSize)
	{
		m_bufferSize = bufferSize;

		if (!emul::IsPowerOf2(bufferSize))
		{
			LogPrintf(LOG_ERROR, "Buffer size is not a power of two: %d", bufferSize);
			return false;
		}

		m_bufSilence = new int16_t[m_bufferSize];
		m_bufPlaying = new int16_t[m_bufferSize];
		m_bufStaging = new int16_t[m_bufferSize];

		LogPrintf(LOG_INFO, "Initialize sound engine - Buffer size: %d", bufferSize);

		if (SDL_WasInit(SDL_INIT_AUDIO) == 0)
		{
			LogPrintf(LOG_INFO, "SDL Init Subsystem [Audio]");
			if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
			{
				LogPrintf(LOG_ERROR, "Error initializing sound audio subsystem: %s", SDL_GetError());
				return false;
			}
		}
		InitSDLAudio();

		return true;
	}

	void Sound::InitSDLAudio()
	{
		SDL_AudioSpec want;
		want.freq = 44100;
		want.format = AUDIO_S16;
		want.channels = 1;
		want.samples = m_bufferSize;
		want.callback = &AudioCallback;
		want.userdata = nullptr;

		m_audioDeviceID = SDL_OpenAudioDevice(0, 0, &want, &m_audioSpec, /*SDL_AUDIO_ALLOW_ANY_CHANGE*/0);

		for (WORD i = 0; i < m_bufferSize; ++i)
		{
			m_bufSilence[i] = m_audioSpec.silence;
			m_bufStaging[i] = m_audioSpec.silence;
			m_bufPlaying[i] = m_audioSpec.silence;
		}

		SDL_PauseAudioDevice(m_audioDeviceID, false);
	}

	void Sound::MoveToPlayingBuffer()
	{
		SDL_LockAudio();
		memcpy(m_bufPlaying, m_bufStaging, m_bufferSize * sizeof(uint16_t));
		SDL_UnlockAudio();
	}

	void Sound::SetMasterVolume(int vol)
	{
		m_masterVolume = std::clamp(vol, 0, SDL_MIX_MAXVOLUME);
		LogPrintf(LOG_INFO, "Set Master Volume [%d]", m_masterVolume);
	}

	uint16_t Sound::PeriodToSamples(float period)
	{
		return (uint16_t)(period * m_audioSpec.freq / 1000000.0);
	}

	void Sound::StreamToFile(bool stream, const char* outFile)
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

	void Sound::Play(WORD data)
	{
		static int sample = 0;
		static int32_t avg = 0;

		// temp hack, "average" 27 samples
		// possibly 4 incoming channels + this one, max possible value = 255*5 = 34425
		// could clip with 4 channels + pc speaker full blast but very unlikely
		if (m_outputFile) fputc(data, m_outputFile);
		avg += data;
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

}
