#pragma once

#include "../Hardware/Device8254.h"
#include "../Hardware/Device8255.h"

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

		void Tick(WORD mixWith = 0);

		void StreamToFile(bool stream, const char* outFile = nullptr);

		const SDL_AudioSpec& GetAudioSpec() const { return m_audioSpec; }

		int16_t* GetPlayingBuffer() const { return m_bufPlaying; }
		int16_t* GetSilenceBuffer() const { return m_bufSilence; }

		bool IsStagingFull() const { return m_bufStagingPos == m_bufferSize; }
		void ResetStaging() { m_bufStagingPos = 0; }

		void SetMute(bool mute) { m_muted = mute; }

		int GetMasterVolume() const { return m_masterVolume; }
		void SetMasterVolume(int vol);

	protected:
		const WORD m_bufferSize;

		uint16_t PeriodToSamples(float period);

		void InitSDLAudio();

		void AddSample(int16_t s) { m_bufStaging[m_bufStagingPos++] = s; }
		void MoveToPlayingBuffer();

		ppi::Device8255* m_8255 = nullptr;
		pit::Device8254* m_8254 = nullptr;

		SDL_AudioSpec m_audioSpec;
		SDL_AudioDeviceID m_audioDeviceID;

		int16_t* m_bufSilence = nullptr;
		int16_t* m_bufPlaying = nullptr;
		int16_t* m_bufStaging = nullptr;
		size_t m_bufStagingPos = 0;

		FILE* m_outputFile = nullptr;

		bool m_muted = false;
		int m_masterVolume = SDL_MIX_MAXVOLUME; // 0..128
	};
}
