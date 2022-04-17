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

		assert(length == spec.samples * sizeof(int16_t) * 2);

		const uint8_t* source = (const uint8_t*)SOUND().GetPlayingBuffer();

		memset(stream, 0, length);
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
		Cleanup();
	}

	void Sound::Cleanup()
	{
		if (m_audioDeviceID)
		{
			SDL_CloseAudioDevice(m_audioDeviceID);
			m_audioDeviceID = 0;
		}

		delete[] m_bufSilence;
		m_bufSilence = nullptr;

		delete[] m_bufPlaying;
		m_bufPlaying = nullptr;

		delete[] m_bufStaging;
		m_bufStaging = nullptr;
	}

	bool Sound::Init(WORD bufferSampleFrames)
	{
		m_bufferSize = bufferSampleFrames;

		if (!emul::IsPowerOf2(bufferSampleFrames))
		{
			LogPrintf(LOG_ERROR, "Buffer size is not a power of two: %d", bufferSampleFrames);
			return false;
		}

		// x2 for stereo
		m_bufSilence = new int16_t[m_bufferSize * 2];
		m_bufPlaying = new int16_t[m_bufferSize * 2];
		m_bufStaging = new int16_t[m_bufferSize * 2];

		LogPrintf(LOG_INFO, "Initialize sound engine - Buffer size: %d sample frames", m_bufferSize);

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
		want.channels = 2; // Stereo
		want.samples = m_bufferSize;
		want.callback = &AudioCallback;
		want.userdata = nullptr;

		m_audioDeviceID = SDL_OpenAudioDevice(0, 0, &want, &m_audioSpec, 0);
		if (m_audioDeviceID == 0)
		{
			LogPrintf(LOG_ERROR, "Unable to open Audio Device: %s", SDL_GetError());
		}

		for (WORD i = 0; i < m_bufferSize * 2; ++i)
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
		memcpy(m_bufPlaying, m_bufStaging, m_bufferSize * sizeof(uint16_t) * 2);
		SDL_UnlockAudio();
	}

	void Sound::SetMasterVolume(int vol)
	{
		m_masterVolume = std::clamp(vol, 0, SDL_MIX_MAXVOLUME);
		LogPrintf(LOG_INFO, "Set Master Volume [%d]", m_masterVolume);
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

	void Sound::PlayMono(int16_t data)
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
			//LogPrintf(LOG_INFO, "pos: %d", m_bufStagingPos);
			if (IsStagingFull())
			{
				MoveToPlayingBuffer();
			}

			// Crude synchronization
			while (IsStagingFull()) { std::this_thread::yield(); };

			avg /= 27;
			AddSample(m_muted ? 0 : avg); // Left
			AddSample(m_muted ? 0 : avg); // Right

			avg = 0;
			sample = 0;
		}
	}

	void Sound::PlayStereo(int16_t left, int16_t right)
	{
		static int sample = 0;
		static int32_t avgL = 0;
		static int32_t avgR = 0;

		// temp hack, "average" 27 samples
		// possibly 4 incoming channels + this one, max possible value = 255*5 = 34425
		// could clip with 4 channels + pc speaker full blast but very unlikely
		if (m_outputFile)
		{
			fputc(left, m_outputFile);
			fputc(right, m_outputFile);
		}
		avgL += left;
		avgR += right;
		++sample;
		if (sample == 27)
		{
			if (IsStagingFull())
			{
				MoveToPlayingBuffer();
			}

			// Crude synchronization
			while (IsStagingFull()) { std::this_thread::yield(); };

			AddSample(m_muted ? 0 : (avgL / 27));
			AddSample(m_muted ? 0 : (avgR / 27));

			avgL = 0;
			avgR = 0;
			sample = 0;
		}
	}

}
