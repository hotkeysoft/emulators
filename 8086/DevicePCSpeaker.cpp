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

	void DevicePCSpeaker::Tick(WORD mixWith)
	{
		static int sample = 0;
		static int32_t avg = 0;

		// temp hack, avg 27 samples
		// possibly 4 incoming channels + this one, max possible value = 255*5, 
		// must divide again by 5 to scale down
		BYTE speakerData = (m_8255->IsSoundON() && m_8254->GetCounter(2).GetOutput()) ? 128 : 0;
		if (m_outputFile) fputc(speakerData, m_outputFile);
		avg += speakerData;
		avg += mixWith;
		++sample;
		if (sample == 27)
		{
			avg /= (27 * 5); // Average
			avg -= 127; // Center around zero

			// Crude synchronization
			while (IsFull()) { std::this_thread::yield(); };

			AddSample(avg);
			avg = 0;
			sample = 0;
		}
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
