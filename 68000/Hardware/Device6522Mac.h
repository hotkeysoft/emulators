#pragma once
#include <Hardware/Device6522.h>

namespace via
{
	class Device6522Mac : public Device6522
	{
	public:
		Device6522Mac(std::string id = "VIA") : Device6522(id), Logger(id.c_str()) {}

		void Init()
		{
			Device6522::Init(false);
		}

		// Port A Outputs
		BYTE GetSoundVolume() { return m_portA.GetOutput() % 7; }
		bool IsSoundBufferPage2() const { return !emul::GetBit(m_portA.GetOutput(), 3); }
		bool IsOverlay() const { return emul::GetBit(m_portA.GetOutput(), 4); }
		bool GetDiskHeadSelect() const { return emul::GetBit(m_portA.GetOutput(), 5); }
		bool IsVideoPage2() const { return !emul::GetBit(m_portA.GetOutput(), 6); }

		// Port A Input
		void SetSCCWriteRequest(bool set) { m_portA.SetInputBit(7, !set); }

		// CA1 VSync Input // TODO: VSync or VBlank?
		void SetVSync(bool set) { m_portA.SetC1(set); }

		// CA2 1-Second clock Input
		void SetSecondClock(bool set) { m_portA.SetC2(set); }

		// Port B Outputs
		bool GetRTCData() const { return emul::GetBit(m_portB.GetOutput(), 0); } // I/O
		bool GetRTCClock() const { return emul::GetBit(m_portB.GetOutput(), 1); }
		bool GetRTCEnable() const { return !emul::GetBit(m_portB.GetOutput(), 2); }
		bool IsSoundReset() const { return !emul::GetBit(m_portB.GetOutput(), 7); }

		// PORT B Inputs
		void SetRTCData(bool set) { m_portB.SetInputBit(0, set); }
		void SetMouseSwitchPressed(bool set) { m_portB.SetInputBit(3, !set); }
		void SetMouseX2(bool set) { m_portB.SetInputBit(4, !set); }
		void SetMouseY2(bool set) { m_portB.SetInputBit(5, !set); }
		void SetHSync(bool set) { m_portB.SetInputBit(6, !set); } // TODO: HSync or HBlank?

		// CB1 Input/Output
		bool GetKeyboardClock() const { return m_portB.GetC1(); }
		void SetKeyboardClock(bool set) { m_portB.SetC1(set); }

		// CB2 Input/Output
		bool GetKeyboardData() const { return m_portB.GetC2(); }
		void SetKeyboardData(bool set) { m_portB.SetC2(set); }

		// Data Registers
		BYTE ReadRegister(WORD reg)
		{
			switch (reg)
			{
			case 0x0: return m_portB.ReadInputRegister();
			case 0x1: return m_portA.ReadInputRegister();
			case 0x2: return m_portB.ReadDataDirectionRegister();
			case 0x3: return m_portA.ReadDataDirectionRegister();
			case 0x4: return ReadT1CounterL();
			case 0x5: return ReadT1CounterH();
			case 0x6: return ReadT1LatchL();
			case 0x7: return ReadT1LatchH();
			case 0x8: return ReadT2CounterL();
			case 0x9: return ReadT2CounterH();
			case 0xA: return ReadSR();
			case 0xB: return ReadACR();
			case 0xC: return ReadPCR();
			case 0xD: return ReadIFR();
			case 0xE: return ReadIER();
			case 0xF: return m_portA.ReadInputRegisterNoHandshake();
			default:
				NODEFAULT;
			}
		}
		void WriteRegister(WORD reg, BYTE value)
		{
			switch (reg)
			{
			case 0x0: m_portB.WriteOutputRegister(value); break;
			case 0x1: m_portA.WriteOutputRegister(value); break;
			case 0x2: m_portB.WriteDataDirectionRegister(value); break;
			case 0x3: m_portA.WriteDataDirectionRegister(value); break;
			case 0x4: WriteT1LatchL(value); break;
			case 0x5: WriteT1CounterH(value); break;
			case 0x6: WriteT1LatchL(value); break;
			case 0x7: WriteT1LatchH(value); break;
			case 0x8: WriteT2LatchL(value); break;
			case 0x9: WriteT2CounterH(value); break;
			case 0xA: WriteSR(value); break;
			case 0xB: WriteACR(value); break;
			case 0xC: WritePCR(value); break;
			case 0xD: WriteIFR(value); break;
			case 0xE: WriteIER(value); break;
			case 0xF: m_portA.WriteOutputRegisterNoHandshake(value); break;
			default:
				NODEFAULT;
			}
		}

	protected:
	};
}

