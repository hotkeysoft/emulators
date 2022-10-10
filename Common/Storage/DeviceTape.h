#pragma once

#include <Serializable.h>
#include <CPU/PortConnector.h>

namespace tape
{
	const size_t MAX_TAPE_COUNT = 2;
	enum class TapeState { STOP = 0, REW, FWD, PLAY, REC };

	class TapeDeck : public Logger
	{
	public:
		TapeDeck(std::string id) : Logger(id.c_str()) {}

		void Init(size_t sampleRate);

		void SetState(TapeState state) { m_state = state; }
		TapeState GetState() const { return m_state; }
		bool GetSense() const { return m_state >= TapeState::PLAY; }

		size_t GetTapeLen() const { return m_data.size(); }

		void New(size_t initialLen = 60 * 10);
		void Load(const char* path);
		void Save(const char* path);
		void Eject();

		void ResetCounter() { m_counter = 0; }
		size_t GetCounterRaw() const { return m_counter; }
		size_t GetCounterSeconds() const { return m_counter; } // TODO

		// Only affects PLAY and REC states
		void SetMotor(bool enable) { m_motorEnabled = enable; }

		// Reads bit at current position
		bool Read() const { return m_currIn; }

		// Writes bit at current position
		void Write(bool val) { m_currOut = val; }

		void Tick();

	protected:
		using RawData = std::vector<bool>;
		RawData m_data;
		size_t m_counter = 0;
		size_t m_fastIncrement = 10;
		size_t m_maxCounter = 0;

		bool m_tapeLoaded = false;
		bool m_motorEnabled = false;
		TapeState m_state = TapeState::STOP;

		bool m_currOut = false;
		bool m_currIn = false;

		size_t m_sampleRate = 0;
	};

	// Common Interface for tape drives
	class DeviceTape : public emul::PortConnector
	{
	public:
		DeviceTape(size_t clockSpeedHz);
		virtual ~DeviceTape() {}

		DeviceTape(const DeviceTape&) = delete;
		DeviceTape& operator=(const DeviceTape&) = delete;
		DeviceTape(DeviceTape&&) = delete;
		DeviceTape& operator=(DeviceTape&&) = delete;

		virtual void Init(size_t maxTapeCount);

		virtual void EnableLog(SEVERITY minSev) override;

		// Number of tape drives
		virtual size_t GetCount() const { return m_deckCount; }

		virtual void Tick();

		TapeDeck& GetTape(int drive)
		{
			assert((drive >= 0) && (drive < m_deckCount));
			return m_decks[drive];
		}

	protected:
		const size_t m_clockSpeedHz;
		size_t m_pollInterval = 0;
		size_t m_cooldown = 0;

		TapeDeck m_decks[MAX_TAPE_COUNT];
		size_t m_deckCount = MAX_TAPE_COUNT;
	};
}
