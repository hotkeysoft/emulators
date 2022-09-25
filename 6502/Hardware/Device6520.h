#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/IOConnector.h>

using emul::IOConnector;
using emul::BYTE;
using emul::WORD;

namespace pia
{
	class Device6520;

	class PIAPort : public IOConnector
	{
	public:
		PIAPort(std::string id);

		void Init(Device6520* parent, bool isPortB);

		void Reset();

		enum DataDirection {INPUT = 0, OUTPUT = 1};

		//TODO: Temporary
		void SetC1(bool set) { emul::SetBit(CR.data, 7, set); }
		void SetC2(bool set) {  emul::SetBit(CR.data, 6, set); }

	protected:
		// CPU IO Access
		// -------------

		// 0 - Read PortData/DataDirectionRegister
		BYTE Read0();
		BYTE ReadPortData();
		BYTE ReadDataDirectionRegister();

		// 0 - Write OR/DDR
		void Write0(BYTE value);
		void WriteOutputRegister(BYTE value);
		void WriteDataDirectionRegister(BYTE value);

		// 1 - Read/Write CR
		BYTE ReadControlRegister();
		void WriteControlRegister(BYTE value);

		// Registers
		// ---------

		// Data direction register
		BYTE DDR = 0;
		DataDirection GetDataDirection(int pin) { return (DataDirection)emul::GetBit(DDR, pin); }

		// Output Register
		BYTE OR = 0; // Output register, set by CPU

		struct ControlRegister
		{
			// Control Register
			BYTE data = 0;

			// Control Registers helpers

			bool GetIRQ1Flag() const { return emul::GetBit(data, 7); }
			bool GetIRQ2Flag() const { return GetC2OutputMode() ? 0 : emul::GetBit(data, 6); }
			void ClearIRQFlags() { data &= 0b00111111; }
			// Read-only?

			// 0: C2 is an input pin, 1: C2 is an output pin
			bool GetC2OutputMode() const { return emul::GetBit(data, 5); }

			// C2 Control (when C2 is in output mode)
			bool GetC2OutputControl() const { return emul::GetBit(data, 4); }
			bool GetC2RestoreControl() const { return emul::GetBit(data, 3); }

			// CPUIRQ/IRQ2 Control (when C2  is in input mode)
			bool GetIRQ2PositiveTransition() const { return emul::GetBit(data, 4); }
			bool GetCPUIRQEnableForIRQ2() const { return emul::GetBit(data, 3); }

			// 0: Select Data Direction Register, 1: Select Output Register
			bool GetORSelect() const { return emul::GetBit(data, 2); }

			// CPUIRQ/IRQ1 Control
			bool GetIRQ1PositiveTransition() const { return emul::GetBit(data, 1); }
			bool GetCPUIRQEnableForIRQ1() const { return emul::GetBit(data, 0); }
		} CR;
		void LogControlRegister();


		// Interrupt status control
		BYTE ISC = 0;

		// Input line
		bool C1 = false;

		// Input/output line
		bool C2 = false;

		// IRQ line to CPU
		bool IRQ = false;
	};

	// 6520 PIA
	class Device6520 : public IOConnector
	{
	public:
		Device6520(std::string id = "PIA");
		virtual ~Device6520() {}

		Device6520(const Device6520&) = delete;
		Device6520& operator=(const Device6520&) = delete;
		Device6520(Device6520&&) = delete;
		Device6520& operator=(Device6520&&) = delete;

		virtual void Init();

		virtual void Reset();

		PIAPort& GetPortA() { return m_portA; }
		PIAPort& GetPortB() { return m_portB; }

	protected:
		PIAPort m_portA;
		PIAPort m_portB;
	};
}
