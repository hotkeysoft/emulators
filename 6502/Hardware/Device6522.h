#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/IOConnector.h>

using emul::IOConnector;
using emul::BYTE;
using emul::WORD;

namespace via
{
	// 6522 VIA
	class Device6522 : public IOConnector
	{
	public:
		Device6522();
		virtual ~Device6522() {}

		//Device6522() = delete;
		Device6522(const Device6522&) = delete;
		Device6522& operator=(const Device6522&) = delete;
		Device6522(Device6522&&) = delete;
		Device6522& operator=(Device6522&&) = delete;

		virtual void Init();

	protected:
		// Reg 0-3: IO ports

		// 0 - ORB/IRB: Output/Input Register B
		BYTE ReadIRB();
		void WriteORB(BYTE value);

		// 1 - ORA/IRA: Output/Input Register A
		BYTE ReadIRA();
		void WriteORA(BYTE value);

		// 2 - DDRB: Data Direction Register B
		BYTE ReadDDRB();
		void WriteDDRB(BYTE value);

		// 3 - DDRA: Data Direction Register A
		BYTE ReadDDRA();
		void WriteDDRA(BYTE value);

		// 4 - T1C-L: T1 Low-Order Counter
		BYTE ReadT1CounterL();

		// 5 - T1C-H: T1 High-Order Counter
		BYTE ReadT1CounterH();
		void WriteT1CounterH(BYTE value);

		// 6 - T1L-L: T1 Low-Order Latches
		BYTE ReadT1LatchesL();
		void WriteT1LatchesL(BYTE value);

		// 7 - T1L-H: T1 High-Order Latches
		BYTE ReadT1LatchesH();
		void WriteT1LatchesH(BYTE value);

		// 8 - T2C-L: T2 Low-Order Counter
		BYTE ReadT2CounterL();
		void WriteT2LatchesL(BYTE value);

		// 9 - T2C-H: T2 High-Order Counter
		BYTE ReadT2CounterH();
		void WriteT2CounterH(BYTE value);

		// A - SR: Shift Register
		BYTE ReadSR();
		void WriteSR(BYTE value);

		// B - ACR: Auxiliary Control Register
		BYTE ReadACR();
		void WriteACR(BYTE value);

		// C - PCR: Peripheral Control Register
		BYTE ReadPCR();
		void WritePCR(BYTE value);

		// D - IFR: Interrupt Flag Register
		BYTE ReadIFR();
		void WriteIFR(BYTE value);

		// E - IER: Interrupt Enable Register
		BYTE ReadIER();
		void WriteIER(BYTE value);

		// F - ORA/IRA: Same as reg 1 with no handshake
		BYTE ReadIRANoHandshake();
		void WriteORANoHandshake(BYTE value);
	};
}
