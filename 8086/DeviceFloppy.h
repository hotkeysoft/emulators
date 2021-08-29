#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Logger.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace fdc
{
	class DeviceFloppy : public PortConnector
	{
	public:
		DeviceFloppy(WORD baseAddress);

		DeviceFloppy() = delete;
		DeviceFloppy(const DeviceFloppy&) = delete;
		DeviceFloppy& operator=(const DeviceFloppy&) = delete;
		DeviceFloppy(DeviceFloppy&&) = delete;
		DeviceFloppy& operator=(DeviceFloppy&&) = delete;

		void Init();
		void Reset();

		void Tick();

		BYTE ReadStatusRegA();
		BYTE ReadStatusRegB();

		void WriteDataOutputReg(BYTE value);

		BYTE ReadTapeReg();
		void WriteTapeReg(BYTE value);

		BYTE ReadMainStatusReg();
		void WriteDataRateSelectReg(BYTE value);

		BYTE ReadDataFIFO();
		void WriteDataFIFO(BYTE value);

		BYTE ReadDigitalInputReg();
		void WriteConfigControlReg(BYTE value);

	protected:
		const WORD m_baseAddress;

		enum DOR
		{
			MOTD  = 0x80, // Set to turn drive 3's motor ON
			MOTC  = 0x40, // Set to turn drive 2's motor ON
			MOTB  = 0x20, // Set to turn drive 1's motor ON
			MOTA  = 0x10, // Set to turn drive 0's motor ON
			IRQ   = 0x08, // Set to enable IRQs and DMA
			RESET = 0x04, // Clear = enter reset mode, Set = normal operation
			DSEL1 = 0x02, // "Select" drive number for next access
			DSEL0 = 0x01,
		};

		bool m_motor3;
		bool m_motor2;
		bool m_motor1;
		bool m_motor0;

		bool m_enableIRQDMA;

		BYTE m_driveSel;
	};
}
