#pragma once

#include "Common.h"
#include "Logger.h"

#include "Device8254.h"
#include "Device8255.h"

#include <SDL.h>

using emul::WORD;

namespace beeper
{	
	class DevicePCSpeaker : public Logger
	{
	public:
		DevicePCSpeaker(WORD bufferSize = 1024);
		~DevicePCSpeaker();

		DevicePCSpeaker(const DevicePCSpeaker&) = delete;
		DevicePCSpeaker& operator=(const DevicePCSpeaker&) = delete;
		DevicePCSpeaker(DevicePCSpeaker&&) = delete;
		DevicePCSpeaker& operator=(DevicePCSpeaker&&) = delete;

		void Init(ppi::Device8255* ppi, pit::Device8254* pit);
		void Reset();

		void Tick();

		const SDL_AudioSpec& GetAudioSpec() const { return m_audioSpec; }

		int8_t* GetPlayingBuffer() const { return m_bufPlaying; }
		int8_t* GetSilenceBuffer() const { return m_bufSilence; }
		bool IsFull() const { return m_bufNextPos == m_bufferSize; }
		void ResetBuffer() { m_bufNextPos = 0; }

	protected:
		const WORD m_bufferSize;

		uint16_t PeriodToSamples(float period);

		void InitSDLAudio();

		void AddSample(int8_t s) { m_bufPlaying[m_bufNextPos++] = s; }

		ppi::Device8255* m_8255 = nullptr;
		pit::Device8254* m_8254 = nullptr;

		SDL_AudioSpec m_audioSpec;
		SDL_AudioDeviceID m_audioDeviceID;

		int8_t* m_bufSilence = nullptr;
		int8_t* m_bufPlaying = nullptr;
		size_t m_bufNextPos = 0;

		int8_t* m_bufNext = nullptr;

	};
}
