#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/IOConnector.h>

using emul::IOConnector;
using emul::BYTE;
using emul::WORD;

namespace via
{
	class VIAPort : public IOConnector
	{
	public:
		VIAPort(std::string id);

		void Init(bool isPortB);

		void Reset();

		enum DataDirection { INPUT = 0, OUTPUT = 1 };

		BYTE GetOutput() const { return OR; }
		void SetInputBit(BYTE bit, bool set) { emul::SetBit(IR, bit, set); }

	protected:
		// CPU IO Access
		// -------------

		// 0 - Read InputRegister (IR)
		BYTE ReadInputRegister();
		// 0 - Write OutputRegister (OR)
		void WriteOutputRegister(BYTE value);

		// 2 - Read/Write DataDirectionRegister (DDR)
		BYTE ReadDataDirectionRegister();
		void WriteDataDirectionRegister(BYTE value);

		// F - Port A: ORA/IRA with no handshake
		BYTE ReadInputRegisterNoHandshake();
		void WriteOutputRegisterNoHandshake(BYTE value);

		// Registers
		// ---------

		// Data direction register
		BYTE DDR = 0;
		DataDirection GetDataDirection(int pin) { return (DataDirection)emul::GetBit(DDR, pin); }

		// Output Register
		BYTE OR = 0; // Output register, set by CPU

		// Input Register
		BYTE IR = 0; // Input set by hardware

		// Input line
		bool C1 = false;

		// Input/output line
		bool C2 = false;
	};

	class Device6522 : public IOConnector
	{
	public:
		Device6522(std::string id = "VIA");
		virtual ~Device6522() {}

		Device6522(const Device6522&) = delete;
		Device6522& operator=(const Device6522&) = delete;
		Device6522(Device6522&&) = delete;
		Device6522& operator=(Device6522&&) = delete;

		virtual void Init();

		virtual void Reset();

		VIAPort& GetPortA() { return m_portA; }
		VIAPort& GetPortB() { return m_portB; }

		bool GetIRQ() const { return false; } // TODO

	protected:
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

	protected:

		struct InterruptEnable
		{
			// Interrupt Enable Register
			BYTE data = 0;

			// Interrupt Enable Register helpers
			void Clear() { data = 0; }
			void Set(BYTE value)
			{
				bool set = emul::GetMSB(value);
				emul::SetBitMask(data, value, set);
				data |= 0x80;
			};

			enum IERFlag { CA2, CA1, SR, CB2, CB1, TIMER2, TIMER1 };
			bool IsInterruptEnabled(IERFlag flag) const { return emul::GetBit(data, (int)flag); }

		} IER;

		struct PeripheralControl
		{
			// Peripheral Control Register
			BYTE data = 0;

			// Peripheral Control Register helpers
			void Clear() { data = 0; }

			enum PCROperation {
				IN_NEG_EDGE = 0,
				IN_NEG_EDGE_INT,
				IN_POS_EDGE,
				IN_POS_EDGE_INT,
				OUT_HANDSHAKE,
				OUT_PULSE,
				OUT_LOW,
				OUT_HIGH
			};
			enum ActiveEdge { NEG_EDGE, POS_EDGE };

			ActiveEdge GetCA1InterruptActiveEdge() const { return (ActiveEdge)emul::GetBit(data, 0); }
			ActiveEdge GetCB1InterruptActiveEdge() const { return (ActiveEdge)emul::GetBit(data, 4); }

			PCROperation GetCA2Operation() const { return (PCROperation)((data >> 1) & 7); }
			PCROperation GetCB2Operation() const { return (PCROperation)((data >> 5) & 7); }

			const char* GetOperationStr(PCROperation op) const;
		} PCR;

		VIAPort m_portA;
		VIAPort m_portB;
	};
}
