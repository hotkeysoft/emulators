#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/IOConnector.h>

using emul::IOConnector;
using emul::BYTE;
using emul::WORD;

namespace pia
{
	// 6520 PIA
	class Device6520 : public IOConnector
	{
	public:
		Device6520(const char* id = "pia");
		virtual ~Device6520() {}

		Device6520(const Device6520&) = delete;
		Device6520& operator=(const Device6520&) = delete;
		Device6520(Device6520&&) = delete;
		Device6520& operator=(Device6520&&) = delete;

		virtual void Init();

		virtual void Reset();

	protected:
		// 0 - Read PIBA/DDRA
		BYTE Read0();
		BYTE ReadPIBA();
		BYTE ReadDDRA();

		// 0 - Write ORA/DDRA
		void Write0(BYTE value);
		void WriteORA(BYTE value);
		void WriteDDRA(BYTE value);

		// 1 - Read/Write CRA
		BYTE ReadCRA();
		void WriteCRA(BYTE value);

		// 2 - Read PIBB/DDRB
		BYTE Read2();
		BYTE ReadPIBB();
		BYTE ReadDDRB();

		// 2 - Write ORB/DDRB
		void Write2(BYTE value);
		void WriteORB(BYTE value);
		void WriteDDRB(BYTE value);

		// 3 - Read/Write CRB
		BYTE ReadCRB();
		void WriteCRB(BYTE value);
	};
}
