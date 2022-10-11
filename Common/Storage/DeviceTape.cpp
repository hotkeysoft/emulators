#include "stdafx.h"

#include <Storage/DeviceTape.h>
#include <Config.h>

using cfg::CONFIG;

namespace tape
{
	void TapeDeck::Init(size_t sampleRate)
	{
		m_sampleRate = sampleRate;
		m_fastIncrement = sampleRate /5000;
		New();
	}

	void TapeDeck::New(size_t initialLen)
	{
		m_tapeLoaded = true;
		m_maxCounter = initialLen * m_sampleRate;
		m_data.resize(m_maxCounter);
		ResetCounter();
		LogPrintf(LOG_INFO, "New Tape, len = %ds (%zu samples)", initialLen, m_maxCounter);
	}

	void TapeDeck::Load(const char* path)
	{

	}

	void TapeDeck::Save(const char* path)
	{

	}

	void TapeDeck::LoadRaw(const char* path, BYTE threshold)
	{
		LogPrintf(LOG_INFO, "LoadRaw, path=%s, threshold=%d", path, threshold);

		std::ifstream inFile(path, std::ios::binary);

		m_data.clear();

		BYTE ch;
		while (inFile.read((char*)&ch, 1))
		{
			static bool lastBit = false;
			bool bit = ch >= threshold ? false : true;
			m_data.push_back(bit);
		}

		LogPrintf(LOG_INFO, "LoadRaw, loaded %d samples", m_data.size());

		inFile.close();
	}

	void TapeDeck::SaveRaw(const char* path)
	{
		std::ofstream outFile(path, std::ios::binary);
		for (auto var : m_data)
		{
			outFile.put(var ? 0xFF : 0);
		}
		outFile.close();
	}

	void TapeDeck::Eject()
	{
		m_tapeLoaded = false;
		m_data.resize(m_sampleRate);
		m_data.clear();
	}

	void TapeDeck::Tick()
	{
		switch (m_state)
		{
		case TapeState::REW:
			if (m_counter > m_fastIncrement)
			{
				m_counter -= m_fastIncrement;
			}
			else
			{
				m_counter = 0;
				m_state = TapeState::STOP;
			}
			break;
		case TapeState::FWD:
			m_counter += m_fastIncrement;
			break;

		case TapeState::PLAY:
			if (m_motorEnabled)
			{
				m_currIn = m_data[m_counter++];
			}
			break;

		case TapeState::REC:
			if (m_motorEnabled)
			{
				m_data[m_counter++] = m_currOut;
			}
			break;

		case TapeState::STOP:
		default:
			// Nothing to do
			break;
		}

		if (m_counter >= m_maxCounter)
		{
			m_counter = m_maxCounter;
			m_state = TapeState::STOP;
		}
	}

	DeviceTape::DeviceTape(size_t clockSpeedHz) :
		Logger("TAPE"),
		m_clockSpeedHz(clockSpeedHz),
		m_decks{ TapeDeck("TAPE.1"), TapeDeck("TAPE.2") }
	{
	}

	void DeviceTape::Init(size_t maxTapeCount)
	{
		EnableLog(CONFIG().GetLogLevel("tape"));

		maxTapeCount = std::min(maxTapeCount, MAX_TAPE_COUNT);
		m_deckCount = CONFIG().GetValueInt32("tape", "count", (int32_t)maxTapeCount);
		m_deckCount = std::min(maxTapeCount, m_deckCount);

		LogPrintf(LOG_INFO, "Max tape deck count:  %d", maxTapeCount);
		LogPrintf(LOG_INFO, "Connected deck count: %d", m_deckCount);

		size_t sampleRate = CONFIG().GetValueInt32("tape", "samplerate", 16000);
		LogPrintf(LOG_INFO, "Sample Rate (req):    %d Hz", sampleRate);

		m_pollInterval = m_clockSpeedHz / sampleRate;
		m_cooldown = m_pollInterval;

		// Adjust for rounding
		sampleRate = m_clockSpeedHz / m_pollInterval;

		LogPrintf(LOG_INFO, "Clock Frequency:      %zi Hz", m_clockSpeedHz);
		LogPrintf(LOG_INFO, "Poll Interval:        %zi", m_pollInterval);
		LogPrintf(LOG_INFO, "Sample Rate (actual): %d Hz", sampleRate);

		for (auto& deck : m_decks)
		{
			deck.Init(sampleRate);
		}
	}

	void DeviceTape::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		for (auto& deck : m_decks)
		{
			deck.EnableLog(minSev);
		}
	}

	void DeviceTape::Tick()
	{
		if (--m_cooldown)
		{
			return;
		}

		m_cooldown = m_pollInterval;

		for (auto& deck : m_decks)
		{
			deck.Tick();
		}
	}
}
