#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Logger.h"
#include <deque>

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
		bool GetSerialOut() const { return m_modemControl.loopback ? true : m_serialOut; }
		void SetSerialIn(BYTE value) { /* TODO, nop in loopback */ }

		// Interrupts
		bool IsInterrupt() const;

		// Outputs pins - positive logic (real pins are active low)
		bool GetRTS() const { return m_modemControl.rts; }
		bool GetDTR() const { return m_modemControl.dtr; }
		bool GetOUT1() const { return m_modemControl.out1; }
		bool GetOUT2() const { return m_modemControl.out2; }

		// Input pins - positive logic (real pins are active low)
		void SetCTS(bool set);
		void SetDSR(bool set);
		void SetDCD(bool set);
		void SetRI(bool set);

		// Transmit clock (/BAUDOUT) is 'hardwired' internally to RCLK
		bool GetBaudOut() const { return false; }

		WORD GetBaudRate() const { return (WORD)(s_clockSpeed / ((size_t)m_divisorLatch * 16)); }

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
		void WriteLineStatus(BYTE value);

		// 6
		BYTE ReadModemStatus();
		void WriteModemStatus(BYTE value);

		struct DataConfig
		{
			BYTE dataLength = 5;
			Parity parity = Parity::NONE;
			StopBits stopBits = StopBits::ONE;
		} m_dataConfig;
		void UpdateDataConfig();

		struct InterruptEnableRegister
		{
			bool dataAvailableInterrupt = false;
			bool txEmpty = false;
			bool errorOrBreak = false;
			bool statusChange = false;
		} m_interruptEnable;

		struct LineControlRegister
		{
			BYTE wordLengthSelect = 0;       // Bit0+1 (0-3 -> 5-8)
			bool stopBits = false;           // Bit2
			bool parityEnable = false;       // Bit3
			bool parityEven = false;         // Bit4
			bool parityStick = false;        // Bit5
			bool setBreak = false;           // Bit6
			bool divisorLatchAccess = false; // Bit7

		} m_lineControl;

		struct ModemControlRegister
		{
			bool dtr = false;      // Bit 0
			bool rts = false;      // Bit 1
			bool out1 = false;     // Bit 2
			bool out2 = false;     // Bit 3
			bool loopback = false; // Bit 4
		} m_modemControl;

		struct LineStatusRegister
		{
			bool dataReady = false;             // Bit 0
			bool overrunError = false;          // Bit 1
			bool parityError = false;           // Bit 2
			bool framingError = false;          // Bit 3
			bool breakInterrupt = false;        // Bit 4
			bool txHoldingRegisterEmpty = true; // Bit 5
			bool txShiftRegisterEmpty = true;   // Bit 6
		} m_lineStatus;

		struct ModemStatusRegister
		{
			bool cts = false; // Bit 4 (delta: Bit 0)
			bool dsr = false; // Bit 5 (delta: Bit 1)
			bool ri = false;  // Bit 6 (delta: Bit 2)
			bool dcd = false; // Bit 7 (delta: Bit 3)
		} m_modemStatus, m_lastModemStatus, m_modemStatusDelta;

		struct INTR
		{
			bool rxLineStatus = false;
			bool rxDataAvailable = false;
			bool txHoldingRegisterEmpty = false;
			bool modemStatus = false;
		} m_intr;

		WORD m_divisorLatch = 0;

		BYTE m_rxBufferRegister;
		BYTE m_txHoldingRegister;
		std::deque<bool> m_txFIFO;
		std::deque<bool> m_rxFIFO;

		bool m_serialOut = true;
	};
}
