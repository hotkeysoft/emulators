#pragma once
#include "../Common.h"
#include "../Serializable.h"
#include "../CPU/PortConnector.h"

namespace seq_ega
{
	struct SequencerData
	{
		struct ClockingMode
		{
			BYTE charWidth = 8; // 8 or 9
			bool lowBandwidth = false; // low: 2/5 mem cycles, hi: 4/5
			bool load16 = false; // 0: serializer load every clock, 1: every 2 clock
			bool halfDotClock = false; // 0: normal dot clock, 1: half (for 320x200 modes)
		} clockingMode;

		BYTE planeMask = 0;
		BYTE charMapSelectA = 0;
		BYTE charMapSelectB = 0;

		struct MemoryMode
		{
			bool alpha = false;
			bool extMemory = false; // 1: allows access to >64k ram
			bool oddEven = false; // 1: Odd/Even access, 0: sequential
		} memoryMode;
	};

	class Sequencer : public emul::PortConnector, public emul::Serializable
	{
	public:
		Sequencer(WORD baseAddress);

		Sequencer() = delete;
		Sequencer(const Sequencer&) = delete;
		Sequencer& operator=(const Sequencer&) = delete;
		Sequencer(Sequencer&&) = delete;
		Sequencer& operator=(Sequencer&&) = delete;

		virtual void Init();
		virtual void Reset();

		const SequencerData& GetData() const { return m_data; }

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		const WORD m_baseAddress;

		enum class SequencerAddress {
			SEQ_RESET = 0,
			SEQ_CLOCKING_MODE,
			SEQ_MAP_MASK,
			SEQ_CHARMAP_SELECT,
			SEQ_MEMORY_MODE,

			SEQ_INVALID
		} m_currAddress = SequencerAddress::SEQ_INVALID;

		void WriteAddress(BYTE value);

		// Dispatches value to functions below
		void WriteValue(BYTE value);
		void WriteReset(BYTE value);

		void WriteClockingMode(BYTE value);
		void WriteMapMask(BYTE value);
		void WriteCharMapSelect(BYTE value);
		void WriteMemoryMode(BYTE value);

		SequencerData m_data;
	};
}

