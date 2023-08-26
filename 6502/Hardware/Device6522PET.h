#pragma once
#include <Hardware/Device6522.h>

namespace via
{
	class Device6522PET : public Device6522
	{
	public:
		Device6522PET(std::string id = "VIA") : Device6522(id), Logger(id.c_str()) {}

		// CA2
		bool GetGraphicsMode() const { return m_portA.GetC2(); }

		// Mix of CB2 and CassetteDataOut
		WORD GetSoundOut() const { return (m_portB.GetC2() ? 8192 : 0) + (GetCassetteDataOut() ? 8192 : 0); }

		// CB1
		void SetCassette2ReadLine(bool set) { return m_portB.SetC1(set); }

		// Port B Inputs
		void SetDAVIn(bool set) { m_portB.SetInputBit(7, !set); }
		void SetNRFDIn(bool set) { m_portB.SetInputBit(6, !set); }
		void SetRetraceIn(bool set) { m_portB.SetInputBit(5, !set); }
		void SetNDACIn(bool set) { m_portB.SetInputBit(0, !set); }

		// Port B Outputs
		bool GetCassette2MotorOut() const { return !emul::GetBit(m_portB.GetOutput(), 4); }
		bool GetCassetteDataOut() const { return !emul::GetBit(m_portB.GetOutput(), 3); }
		bool GetATNOut() const { return !emul::GetBit(m_portB.GetOutput(), 2); }
		bool GetNRFDOut() const { return !emul::GetBit(m_portB.GetOutput(), 1); }

		// TODO: Port A: User port
	};
}

