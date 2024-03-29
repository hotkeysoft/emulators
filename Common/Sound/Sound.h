#pragma once

#include <SDL.h>
#include <FileUtil.h>

using emul::WORD;

namespace sound
{
	// Singleton
	class Sound : public Logger
	{
	public:
		~Sound();

		Sound(const Sound&) = delete;
		Sound& operator=(const Sound&) = delete;
		Sound(Sound&&) = delete;
		Sound& operator=(Sound&&) = delete;

		static Sound& Get();

		// A sample frame is a chunk of audio data of the size specified in format multiplied by the number of channels
		bool Init(WORD bufferSampleFrames = 1024);
		void Cleanup();

		// Used for synchronization, set the interval at which Play() is called
		void SetBaseClock(int freq);

		void StreamToFile(bool stream, const char* outFile = nullptr);

		const SDL_AudioSpec& GetAudioSpec() const { return m_audioSpec; }

		int16_t* GetPlayingBuffer() const { return m_bufPlaying; }
		int16_t* GetSilenceBuffer() const { return m_bufSilence; }

		bool IsStagingFull() const { return m_bufStagingPos == m_bufferSize * 2; }
		void ResetStaging() { m_bufStagingPos = 0; }

		void SetMute(bool mute) { m_muted = mute; }

		int GetMasterVolume() const { return m_masterVolume; }
		void SetMasterVolume(int vol);

		void PlayMono(int16_t data);
		void PlayStereo(int16_t left, int16_t right);

	protected:
		Sound();
		WORD m_bufferSize;

		void InitSDLAudio();

		void AddSample(int16_t s) { m_bufStaging[m_bufStagingPos++] = s; }
		void MoveToPlayingBuffer();

		SDL_AudioSpec m_audioSpec;
		SDL_AudioDeviceID m_audioDeviceID = 0;

		int16_t* m_bufSilence = nullptr;
		int16_t* m_bufPlaying = nullptr;
		int16_t* m_bufStaging = nullptr;
		size_t m_bufStagingPos = 0;

		hscommon::fileUtil::File m_outputFile;

		const int PLAYBACK_FREQUENCY = 44100;
		int m_freqDivider = 1;

		bool m_muted = false;
		int m_masterVolume = SDL_MIX_MAXVOLUME; // 0..128
	};

	constexpr auto SOUND = &Sound::Get;
}
