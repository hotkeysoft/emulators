#pragma once

#include <Serializable.h>
#include <CPU/CPUCommon.h>
#include <CPU/IOConnector.h>

using emul::IOConnector;
using emul::BYTE;
using emul::WORD;

namespace via
{
	enum class C2Operation {
		IN_NEG_EDGE = 0,
		IN_NEG_EDGE_INT,
		IN_POS_EDGE,
		IN_POS_EDGE_INT,
		OUT_HANDSHAKE,
		OUT_PULSE,
		OUT_LOW,
		OUT_HIGH
	};
	enum class ShiftRegisterMode {
		DISABLED = 0,
		SHIFT_IN_T2,
		SHIFT_IN_CLK,
		SHIFT_IN_EXTCLK,
		SHIFT_OUT_T2_FREE,
		SHIFT_OUT_T2,
		SHIFT_OUT_CLK,
		SHIFT_OUT_EXTCLK
	};
	enum class ActiveEdge { NEG_EDGE, POS_EDGE };
	enum class InterruptFlag { CA2, CA1, SR, CB2, CB1, TIMER2, TIMER1, ANY };

	class VIAPort : public IOConnector, public emul::Serializable
	{
	public:
		VIAPort(std::string id);

		void Init(bool isPortB);

		void Reset();

		enum DataDirection { INPUT = 0, OUTPUT = 1 };

		BYTE GetOutput() const { return OR; }
		void SetInputBit(BYTE bit, bool set) { emul::SetBit(IR, bit, set); }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

		bool GetC1() const { return C1; }
		bool GetC2() const { return C2; }

		void SetC1(bool set) { C1 = set; }
		void SetC2(bool set) { C2 = set; }

		void SetC2Operation(C2Operation op);

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

	class Device6522 : public IOConnector, public emul::Serializable
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
		virtual void EnableLog(SEVERITY minSev = LOG_INFO) override;

		VIAPort& GetPortA() { return m_portA; }
		VIAPort& GetPortB() { return m_portB; }

		bool GetIRQ() const { return m_interrupt.IsIRQ(); }

		void Tick();

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		// 4 - T1C-L: T1 Low-Order Counter
		BYTE ReadT1CounterL();

		// 5 - T1C-H: T1 High-Order Counter
		BYTE ReadT1CounterH();
		void WriteT1CounterH(BYTE value);

		// 6 - T1L-L: T1 Low-Order Latch
		BYTE ReadT1LatchL();
		void WriteT1LatchL(BYTE value);

		// 7 - T1L-H: T1 High-Order Latch
		BYTE ReadT1LatchH();
		void WriteT1LatchH(BYTE value);

		// 8 - T2C-L: T2 Low-Order Counter
		BYTE ReadT2CounterL();
		void WriteT2LatchL(BYTE value);

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

		struct Interrupt : emul::Serializable
		{
			// Interrupt Flag Register helpers
			void Clear() { m_interruptEnable = 0; m_interruptFlags = 0; }
			BYTE GetIER() const { return m_interruptEnable | 0x80; }
			BYTE GetIFR() const;

			void SetInterrupt(InterruptFlag flag) { emul::SetBit(m_interruptFlags, (int)flag, true); }
			void ClearInterrupt(InterruptFlag flag) { emul::SetBit(m_interruptFlags, (int)flag, false); }

			bool IsInterruptEnabled(InterruptFlag flag) const { return emul::GetBit(m_interruptEnable, (int)flag); }
			bool IsInterruptSet(InterruptFlag flag) const { return emul::GetBit(GetIFR(), (int)flag); }

			bool IsIRQ() const { return (m_interruptEnable & m_interruptFlags) != 0; }

			// Clears interrupt flag for set bits
			void SetIFR(BYTE value)
			{
				emul::SetBit(value, 7, false);
				emul::SetBitMask(m_interruptFlags, value, false);
			};
			void SetIER(BYTE value)
			{
				bool set = emul::GetMSB(value);
				emul::SetBitMask(m_interruptEnable, value, set);
				emul::SetBit(m_interruptEnable, 7, false);
			};

			// emul::Serializable
			virtual void Serialize(json& to) override;
			virtual void Deserialize(const json& from) override;

		private:
			BYTE m_interruptEnable = 0;
			BYTE m_interruptFlags = 0;
		} m_interrupt;
		void UpdateIER();
		void UpdateIFR();

		struct PeripheralControl
		{
			// Peripheral Control Register helpers
			void Clear() { m_data = 0; }
			BYTE Get() const { return m_data; }
			void Set(BYTE data) { m_data = data; }

			ActiveEdge GetCA1InterruptActiveEdge() const { return (ActiveEdge)emul::GetBit(m_data, 0); }
			ActiveEdge GetCB1InterruptActiveEdge() const { return (ActiveEdge)emul::GetBit(m_data, 4); }

			C2Operation GetCA2Operation() const { return (C2Operation)((m_data >> 1) & 7); }
			C2Operation GetCB2Operation() const { return (C2Operation)((m_data >> 5) & 7); }

		private:
			BYTE m_data = 0;
		} PCR;
		void UpdatePCR();

		struct AuxControl
		{
			// Auxiliary Control Register helpers
			void Clear() { m_data = 0; }
			BYTE Get() const { return m_data; }
			void Set(BYTE data) { m_data = data; }

			// if false, timer generates IRQ only
			// if true, timer outputs signal on PB7 (one shot or square wave)
			bool GetPB7TimerOutput() const { return emul::GetBit(m_data, 7); }

			enum class T1Mode { ONE_SHOT, CONTINUOUS };
			T1Mode GetTimer1Mode() const { return (T1Mode)emul::GetBit(m_data, 6); }

			enum class T2Mode { TIMED_INTERRUPT, PULSE_PB6 };
			T2Mode GetTimer2Mode() const { return (T2Mode)emul::GetBit(m_data, 5); }

			ShiftRegisterMode GetShiftRegisterMode() const { return (ShiftRegisterMode)((m_data >> 2) & 7); }

			bool GetPortBLatchingEnabled() const { return emul::GetBit(m_data, 1); }
			bool GetPortALatchingEnabled() const { return emul::GetBit(m_data, 0); }

		private:
			BYTE m_data = 0;
		} ACR;
		void UpdateACR();

		// TODO: Only one shot, timer1&2 differences
		struct Timer : emul::Serializable
		{
			void Reset();
			bool Tick(); // Returns true when interrupt should be set. TODO: Ugly

			BYTE GetCounterHigh() const { return emul::GetHByte(m_counter); }
			BYTE GetCounterLow() const { return emul::GetLByte(m_counter); }

			BYTE GetCounterHighLatch() const { return emul::GetHByte(m_latch); }
			BYTE GetCounterLowLatch() const { return emul::GetLByte(m_latch); }

			void SetCounterLowLatch(BYTE value) { emul::SetLByte(m_latch, value); }
			void SetCounterHighLatch(BYTE value) { emul::SetHByte(m_latch, value); }

			void Load() { m_load = true; m_armed = true; }
			bool IsArmed() const { return m_armed; }
			void Disarm() { m_armed = false; }

			// emul::Serializable
			virtual void Serialize(json& to) override;
			virtual void Deserialize(const json& from) override;

		private:
			bool m_armed = false;
			bool m_load = false;
			WORD m_latch = 0;
			WORD m_counter = 0;
		} TIMER1, TIMER2;

		void Shift();
		BYTE m_shiftRegister = 0;

		VIAPort m_portA;
		VIAPort m_portB;
	};
}
