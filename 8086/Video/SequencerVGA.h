#pragma once
#include "../Common.h"
#include "../Serializable.h"
#include "../CPU/PortConnector.h"

namespace seq_vga
{
	struct SequencerData
	{
		struct Reset
		{
			bool sync = false;
			bool async = false;
		} reset;

		struct ClockingMode
		{
			BYTE charWidth = 8; // 8 or 9
			bool lowBandwidth = false; // Not used in vga
			bool load16 = false; // 0 (&& !load32): serializer load every clock, 1: every 2 clock
			bool halfDotClock = false; // 0: normal dot clock, 1: half (for 320x200 modes)
			bool load32 = false; // 0 (&& !load16): serializer load every clock, 1: every 4 clock
			bool screenOff = false; // 0: normal, 1: turn off display and gives all bw to cpu
		} clockingMode;

		BYTE planeMask = 0;
		BYTE charMapSelectA = 0;
		BYTE charMapSelectB = 0;

		struct MemoryMode
		{			
			bool extMemory = false; // 1: allows access to >64k ram
			bool oddEven = false; // 1: Odd/Even access, 0: sequential
			bool chain4 = false;  // 1: A1A0 selects plane, 0: sequential
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

		void ConnectPorts();
		void DisconnectPorts();

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

		BYTE ReadAddress();
		void WriteAddress(BYTE value);

		// Dispatches value to functions below
		BYTE ReadValue();
		void WriteValue(BYTE value);

		BYTE ReadReset();
		void WriteReset(BYTE value);

		BYTE ReadClockingMode();
		void WriteClockingMode(BYTE value);

		BYTE ReadMapMask();
		void WriteMapMask(BYTE value);

		BYTE ReadCharMapSelect();
		void WriteCharMapSelect(BYTE value);

		BYTE ReadMemoryMode();
		void WriteMemoryMode(BYTE value);

		SequencerData m_data;
	};
}

