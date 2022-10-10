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
	class Device6520;

	class PIAPort : public IOConnector, public emul::Serializable
	{
	public:
		PIAPort(std::string id);

		void Init(Device6520* parent, bool isPortB);

		void Reset();

		enum DataDirection {INPUT = 0, OUTPUT = 1};

		bool GetC2() const { return !CR.GetC2Output(); }

		void SetC1(bool set)
		{
			CR.IRQ1Latch.Set(set);
		}

		void SetC2(bool set)
		{
			CR.IRQ2Latch.Set(set);
		}

		bool GetIRQ() const {
			return
				(CR.GetCPUIRQEnableForIRQ1() && CR.IRQ1Latch.IsLatched()) ||
				(CR.GetCPUIRQEnableForIRQ2() && CR.IRQ2Latch.IsLatched());
		}

		BYTE GetOutput() const { return OR; }
		void SetInputBit(BYTE bit, bool set) { emul::SetBit(IR, bit, set); }
		void SetInput(BYTE data) { IR = data; }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		Device6520* m_parent = nullptr;
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

		// Input Register
		BYTE IR = 0; // Input set by hardware

		struct ControlRegister : emul::Serializable
		{
			// Control Register
			BYTE data = 0;

			// Control Registers helpers

			bool GetIRQ1Flag() const { return IRQ1Latch.IsLatched(); }
			bool GetIRQ2Flag() const { return GetC2OutputMode() ? 0 : IRQ2Latch.IsLatched(); }
			void ClearIRQFlags() { IRQ1Latch.ResetLatch(); IRQ2Latch.ResetLatch(); }

			// 0: C2 is an input pin, 1: C2 is an output pin
			bool GetC2OutputMode() const { return emul::GetBit(data, 5); }

			// C2 Control (when C2 is in output mode)
			bool GetC2OutputControl() const { return emul::GetBit(data, 4); }
			// When GetC2OutputControl == 0 (not supported)
			bool GetC2RestoreControl() const { return emul::GetBit(data, 3); }
			// When GetC2OutputControl == 1
			bool GetC2Output() const { return emul::GetBit(data, 3); }

			// CPUIRQ/IRQ2 Control (when C2  is in input mode)
			bool GetIRQ2PositiveTransition() const { return emul::GetBit(data, 4); }
			bool GetCPUIRQEnableForIRQ2() const { return emul::GetBit(data, 3); }

			// 0: Select Data Direction Register, 1: Select Output Register
			bool GetORSelect() const { return emul::GetBit(data, 2); }

			// CPUIRQ/IRQ1 Control
			bool GetIRQ1PositiveTransition() const { return emul::GetBit(data, 1); }
			bool GetCPUIRQEnableForIRQ1() const { return emul::GetBit(data, 0); }

			hscommon::EdgeDetectLatch IRQ1Latch;
			hscommon::EdgeDetectLatch IRQ2Latch;

			// emul::Serializable
			virtual void Serialize(json& to) override;
			virtual void Deserialize(const json& from) override;
		} CR;
		void LogControlRegister();

		// Interrupt status control
		BYTE ISC = 0;
	};

	// 6520 PIA
	class Device6520 : public IOConnector, public emul::Serializable
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
		virtual void EnableLog(SEVERITY minSev = LOG_INFO) override;

		PIAPort& GetPortA() { return m_portA; }
		PIAPort& GetPortB() { return m_portB; }

		bool GetIRQA() const { return m_portA.GetIRQ(); }
		bool GetIRQB() const { return m_portB.GetIRQ(); }

		virtual void OnReadPort(PIAPort* src) {};

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		PIAPort m_portA;
		PIAPort m_portB;
	};
}
