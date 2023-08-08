#pragma once

#include <Computer/ComputerBase.h>
#include <CPU/IOBlock.h>
#include "IO/InputEvents.h"

namespace emul
{
	class CPU6809;

	class ComputerThomson : public ComputerBase, public IOConnector
	{
	public:
		ComputerThomson();

		virtual std::string_view GetName() const override { return "Thomson"; };
		virtual std::string_view GetID() const override { return "thomson"; };

		virtual void Init(WORD baseRAM) override;
		virtual void Reset() override;
		virtual bool Step() override;

		CPU6809& GetCPU() const { return *((CPU6809*)m_cpu); }

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitROM();
		void InitRAM();
		void InitIO();
		void InitVideo();

		// TEMP until video
		void DrawScreen();

		// TEMP until PIA is connected
		BYTE ReadIO();
		void WriteIO(BYTE value);

		bool m_forme = true;
		void MapScreenMem();

		// System PIA
		BYTE m_portA = 0xFF;
		BYTE ReadPortA();
		void WritePortA(BYTE value);
		BYTE ReadPortB();
		void WritePortB(BYTE value);

		BYTE ReadControlA();
		void WriteControlA(BYTE value);
		BYTE ReadControlB();
		void WriteControlB(BYTE value);


		void DumpRAM();

		// TODO: Move to video class?
		emul::MemoryBlock m_pixelRAM;
		emul::MemoryBlock m_attributeRAM;

		emul::MemoryBlock m_userRAM;
		emul::MemoryBlock m_osROM;
		emul::MemoryBlock m_basicROM;

		emul::IOBlock m_ioA7C0;
	};
}
