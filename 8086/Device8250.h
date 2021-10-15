#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Logger.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace uart
{
	static size_t s_clockSpeed = 1000000;

	enum class StopBits
	{
		ONE = '1',
		ONE_AND_HALF = 'H',
		TWO = '2'
	};
	enum class Parity
	{
		NONE  = 'N',
		ODD   = 'O',
		EVEN  = 'E',
		MARK  = 'M', // 1
		SPACE = 'S' // 0
	};

	class Device8250 : public PortConnector
	{
	public:
		Device8250(WORD baseAddress, size_t clockSpeedHz = 1000000);

		Device8250() = delete;
		Device8250(const Device8250&) = delete;
		Device8250& operator=(const Device8250&) = delete;
		Device8250(Device8250&&) = delete;
		Device8250& operator=(Device8250&&) = delete;

		void Init();
		void Reset();

		void Tick();

		// Serial in/out
		bool GetSerialOut() const { return m_io.serialOut; }

		// Interrupts
		bool IsInterrupt() const { return false; }

		// Outputs pins - positive logic (real pins are active low)
		bool GetRTS() const { return m_io.rts; }
		bool GetDTR() const { return m_io.dtr; }
		bool GetOUT1() const { return m_io.out1; }
		bool GetOUT2() const { return m_io.out2; }

		// Input pins - positive logic (real pins are active low)
		void SetCTS(bool set) {};
		void SetDSR(bool set) {};
		void SetDCD(bool set) {};
		void SetRI(bool set) {};

		// Transmit clock (/BAUDOUT) is 'hardwired' internally to RCLK
		bool GetBaudOut() const { return false; }

		BYTE GetDataLength() const { return m_dataConfig.dataLength; }
		Parity GetParity() const { return m_dataConfig.parity; }
		StopBits GetStopBits() const { return m_dataConfig.stopBits; }

	protected:
		const WORD m_baseAddress;

		BYTE Read0();
		void Write0(BYTE value);

		BYTE Read1();
		void Write1(BYTE value);

		// 0 && DLAB == 1
		BYTE ReadDivisorLatchLSB();
		void WriteDivisorLatchLSB(BYTE value);

		// 1 && DLAB == 1
		BYTE ReadDivisorLatchMSB();
		void WriteDivisorLatchMSB(BYTE value);

		// 0 && DLAB == 0
		BYTE ReadReceiver();
		void WriteTransmitter(BYTE value);

		// 1 && DLAB == 0
		BYTE ReadInterruptEnable();
		void WriteInterruptEnable(BYTE value);

		// 2
		BYTE ReadInterruptIdentification();

		// 3
		BYTE ReadLineControl();
		void WriteLineControl(BYTE value);

		// 4
		BYTE ReadModemControl();
		void WriteModemControl(BYTE value);

		// 5
		BYTE ReadLineStatus();

		// 6
		BYTE ReadModemStatus();

		// 7
		BYTE ReadScratch();
		void WriteScratch(BYTE value);

		struct DataConfig
		{
			BYTE dataLength = 5;
			Parity parity = Parity::NONE;
			StopBits stopBits = StopBits::ONE;
		} m_dataConfig;
		void UpdateDataConfig();

		struct LineControlRegister
		{
			BYTE wordLengthSelect = 0; //Bit0-Bit1 (0-3 -> 5-8)
			bool stopBits = false; // Bit2
			bool parityEnable = false; // Bit3
			bool parityEven = false; // Bit4
			bool parityStick = false; // Bit5
			bool setBreak = false; // Bit6
			bool divisorLatchAccess = false; // Bit7

		} m_lineControl;

		struct UARTRegisters
		{
			BYTE interruptEnable = 0;
			BYTE modemControl = 0;
			BYTE lineStatus = 0b01100000;
			BYTE modemStatus = 0;
			BYTE scratch = 0;
		} m_reg;

		struct INTR
		{
			bool receiverError = false;
			bool receiverDataAvailable = false;
			bool transmitterEmpty = false;
			bool modemStatus = false;
		} m_intr;

		WORD m_divisorLatch = 0;

		struct IO
		{
			bool out1 = false;
			bool out2 = false;
			bool rts = false;
			bool dtr = false;
			bool serialOut = true;
		} m_io;
	};
}
