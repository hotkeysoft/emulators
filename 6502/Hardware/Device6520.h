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
	protected:
		// 0 - Read PIB/DDR
		BYTE Read0();
		BYTE ReadPIB();
		BYTE ReadDDR();

		// 0 - Write OR/DDR
		void Write0(BYTE value);
		void WriteOR(BYTE value);
		void WriteDDR(BYTE value);

		// 1 - Read/Write CR
		BYTE ReadCR();
		void WriteCR(BYTE value);

		// Data direction register
		BYTE DDR = 0;
		// Output Register
		BYTE OR = 0;
		// Control Register
		BYTE CR = 0;

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

	protected:
		PIAPort m_portA;
		PIAPort m_portB;
	};
}
