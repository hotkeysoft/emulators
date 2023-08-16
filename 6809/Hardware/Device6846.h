#pragma once

#include <Serializable.h>
#include <CPU/CPUCommon.h>
#include <CPU/IOConnector.h>
#include "../EdgeDetectLatch.h"

using emul::IOConnector;
using emul::BYTE;
using emul::WORD;

namespace pia
{
	class Device6846 : public IOConnector, public emul::Serializable
	{
	public:
		Device6846(std::string id);

		void Init();

		void Reset();

		enum class DataDirection {INPUT = 0, OUTPUT = 1};

		void SetCP1(bool set)
		{
			PCR.CP1Latch.Set(set);
		}

		void SetC2(bool set)
		{
			PCR.CP2Latch.Set(set);
		}

		bool GetCP2() const { return PCR.GetCP2Output(); }

		bool GetIRQ() const {
			return
				// TODO: TCR
				(PCR.GetCP1InterruptEnabled() && PCR.CP1Latch.IsLatched()) ||
				(PCR.GetCP2InterruptEnabled() && PCR.CP2Latch.IsLatched());
		}

		BYTE GetOutput() const { return OR; }
		void SetInputBit(BYTE bit, bool set) { emul::SetBit(IR, bit, set); }
		void SetInput(BYTE data) { IR = data; }

		virtual void OnReadPort() {};
		virtual void OnWritePort() {};

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		// CPU IO Access
		// -------------

		// 0,4 - Composite Status Register (CSR)
		BYTE ReadStatus();

		// 1 - Peripherical Control Register (PCR)
		BYTE ReadControl();
		void WriteControl(BYTE value);

		// 2 - Data Direction Register (DDR)
		BYTE ReadDirection();
		void WriteDirection(BYTE value);

		// 3 - Peripherical Data Register (PDR)
		BYTE ReadData();
		void WriteData(BYTE value);

		// 4 == 0

		// 5 - Timer Control Register (TCR)
		BYTE ReadTimerControl();
		void WriteTimerControl(BYTE value);

		// 6 - Timer MSB
		BYTE ReadTimerMSB();
		void WriteTimerMSB(BYTE value);

		// 7 - Timer LSB
		BYTE ReadTimerLSB();
		void WriteTimerLSB(BYTE value);

		// Registers
		// ---------

		// Data direction register
		BYTE DDR = 0;
		DataDirection GetDataDirection(int pin) { return (DataDirection)emul::GetBit(DDR, pin); }

		// Output Register
		BYTE OR = 0; // Output register, set by CPU

		// Input Register
		BYTE IR = 0; // Input set by hardware

		struct PeripheralControlRegister : public emul::Serializable
		{
			void Reset()
			{
				data = 0x80;
				ClearInterruptFlags(true);
			}

			// Peripheral Control Register
			BYTE data = 0;

			// Control Registers helpers

			// PCR7: Reset. 0 = normal operation, 1 = reset
			bool IsReset() const { return emul::GetBit(data, 7); }

			bool GetCP1InterruptFlag() const { return CP1Latch.IsLatched(); }
			bool GetCP2InterruptFlag() const { return (GetCP2Direction() == DataDirection::OUTPUT) ? false : CP2Latch.IsLatched(); }
			void ClearInterruptFlags(bool force = false);

			// PCR5: 0: CP2 is an input pin, 1: CP2 is an output pin
			DataDirection GetCP2Direction() const { return (DataDirection)emul::GetBit(data, 5); }

			// PCR4: CP2 Control (when CP2 is in OUTPUT mode)
			bool GetCP2OutputControl() const { return emul::GetBit(data, 4); }
			// PCR3 When GetCP2OutputControl == 0 (not supported)
			bool GetCP2RestoreControl() const { return emul::GetBit(data, 3); }
			// PCR3 When GetCP2OutputControl == 1
			bool GetCP2Output() const { return emul::GetBit(data, 3); }

			// PCR4,3: CP2 Interrupt Control (when CP2 is in INPUT mode)
			bool GetCP2PositiveTransition() const { return emul::GetBit(data, 4); }
			bool GetCP2InterruptEnabled() const { return emul::GetBit(data, 3); }

			// PCR1: CP1 Input Latch Control (not implemented)
			// 0: Not Latched, 1: Latched on active CP1
			bool GetCP1InputLatchControl() const { return emul::GetBit(data, 2); }

			// PCR1,0: CP2 Interrupt Control (when CP2 is in INPUT mode)
			bool GetCP1PositiveTransition() const { return emul::GetBit(data, 1); }
			bool GetCP1InterruptEnabled() const { return emul::GetBit(data, 0); }

			hscommon::EdgeDetectLatch CP1Latch;
			hscommon::EdgeDetectLatch CP2Latch;
			bool interruptAcknowledged = false;

			// emul::Serializable
			virtual void Serialize(json& to) override;
			virtual void Deserialize(const json& from) override;
		} PCR;
		void LogPeripheralControlRegister() const;

		struct TimerControlRegister : public emul::Serializable
		{
			void Reset()
			{
				data = 1;
				//ClearInterruptFlags(true);
			}

			// Peripheral Control Register
			BYTE data = 1;

			// Timer Register helpers

			// TCR 7: Timer Output Enable: 0 = output set low, 1 = counter output
			bool IsTimerOutputEnabled() const { return emul::GetBit(data, 7); }

			// TCR 6: Timer Interrupt Enable: 0 = IRQ masked, 1 = IRQ enabled
			bool IsTimerInterruptEnabled() const { return emul::GetBit(data, 6); }

			// TCR 5-3: Operating Mode
			// 0-7, very partially implemented, see cpp file
			int GetTimerMode() const { return ((data >> 3) & 7); }

			// TCR2: /8 Prescaler. 0 = Not prescaled, 1 = / 8
			bool IsDiv8Prescaler() const { return emul::GetBit(data, 2); }

			// TCR1: Clock Source. 0 = External clock (CTC), 1 = System Clock (E)
			// External clock not implemented, uses E (Tick())
			bool IsClockSourceSystem() const { return emul::GetBit(data, 1); }

			// TCR0: Internal Reset. 0 = timer enabled, 1 = timer in preset state
			bool IsTimerPreset() const { return emul::GetBit(data, 0); }

			// emul::Serializable
			virtual void Serialize(json& to) override;
			virtual void Deserialize(const json& from) override;
		} TCR;
		void LogTimerControlRegister() const;

		// Timer

		// Temporarily holds MSB until LSB is written (so that latch is set at once)
		BYTE m_tempMSB = 0;
		WORD m_counterLatch = 0;
		WORD m_counter = 0;
	};
}
