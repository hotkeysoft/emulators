#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Logger.h"
#include <deque>

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace fdc
{
	class DeviceFloppy : public PortConnector
	{
	public:
		DeviceFloppy(WORD baseAddress, size_t clockSpeedHz);

		DeviceFloppy() = delete;
		DeviceFloppy(const DeviceFloppy&) = delete;
		DeviceFloppy& operator=(const DeviceFloppy&) = delete;
		DeviceFloppy(DeviceFloppy&&) = delete;
		DeviceFloppy& operator=(DeviceFloppy&&) = delete;

		void Init();
		void Reset();

		size_t DelayToTicks(size_t delayMS);

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

		bool IsInterruptPending() const { return m_interruptPending; }
		void ClearInterrupt() { m_interruptPending = false; }

	protected:
		const WORD m_baseAddress;
		size_t m_clockSpeed;
		size_t m_currOpWait;

		enum class STATE
		{
			RESET_START,
			RESET_ACTIVE,
			RESET_DONE,

			CMD_WAIT,
			CMD_READ,
		} m_state;

		bool m_interruptPending;
		void SetInterruptPending() { m_interruptPending = true; }

		// Status
		enum MSR
		{
			MRQ  = 0x80, // 1 = Data Register Ready
			DIO  = 0x40, // 1 = Controller->CPU, 0 = CPU->Controller
			NDMA = 0x20, // 1 = NON-DMA Mode, 0 = DMA Mode
			BUSY = 0x10, // 1 = Device Busy
			ACTD = 0x08, // 1 = Drive 3 Seeking
			ACTC = 0x04, // 1 = Drive 2 Seeking
			ACTB = 0x02, // 1 = Drive 1 Seeking 
			ACTA = 0x01, // 1 = Drive 0 Seeking
		};

		enum class DataDirection { FDC2CPU, CPU2FDC };

		bool m_commandBusy;
		bool m_driveActive[4];
		DataDirection m_dataInputOutput;
		bool m_dataRegisterReady;

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

		// Drive Output Register
		bool m_motor[4];
		bool m_enableIRQDMA;
		BYTE m_driveSel;

		std::deque<BYTE> m_fifo;

		enum CMD
		{
			READ_TRACK = 2,
			SPECIFY = 3,
			SENSE_DRIVE_STATUS = 4,
			WRITE_DATA = 5,
			READ_DATA = 6,
			RECALIBRATE = 7,
			SENSE_INT_STATUS = 8,
			WRITE_DELETED_DATA = 9,
			READ_ID = 10,
			READ_DELETED_DATA = 12,
			FORMAT_TRACK = 13,
			SEEK = 15,
			SCAN_EQUAL = 17,
			SCAN_LOW_OR_EQUAL = 25,
			SCAN_HIGH_OR_EQUAL = 29
		};

		typedef void (DeviceFloppy::* ExecFunc)(CMD);
		void NotImplemented(CMD id) { LogPrintf(LOG_WARNING, "Exec id=%d Not Implemented", id); }

		struct Command
		{
			const char* name;
			size_t paramCount;
			ExecFunc func;
		};

		typedef std::map<CMD, Command> CommandMap;
		const CommandMap m_commandMap = {
			{ CMD::READ_TRACK,         { "ReadTrack"       , 8, &DeviceFloppy::NotImplemented } },
			{ CMD::SPECIFY,            { "Specify"         , 3, &DeviceFloppy::NotImplemented } },
			{ CMD::SENSE_DRIVE_STATUS, { "SenseDriveStatus", 1, &DeviceFloppy::NotImplemented } },
			{ CMD::WRITE_DATA,         { "WriteData"       , 8, &DeviceFloppy::NotImplemented } },
			{ CMD::READ_DATA,          { "ReadData"        , 8, &DeviceFloppy::NotImplemented } },
			{ CMD::RECALIBRATE,        { "Recalibrate"     , 1, &DeviceFloppy::NotImplemented } },
			{ CMD::SENSE_INT_STATUS,   { "SenseInterrupt"  , 0, &DeviceFloppy::NotImplemented } },
			{ CMD::WRITE_DELETED_DATA, { "WriteDeletedData", 8, &DeviceFloppy::NotImplemented } },
			{ CMD::READ_ID,            { "ReadID"          , 1, &DeviceFloppy::NotImplemented } },
			{ CMD::READ_DELETED_DATA,  { "ReadDeletedData" , 8, &DeviceFloppy::NotImplemented } },
			{ CMD::FORMAT_TRACK,       { "FormatTrack"     , 5, &DeviceFloppy::NotImplemented } },
			{ CMD::SEEK,               { "Seek"            , 2, &DeviceFloppy::NotImplemented } },
			{ CMD::SCAN_EQUAL,         { "ScalEqual"       , 8, &DeviceFloppy::NotImplemented } },
			{ CMD::SCAN_LOW_OR_EQUAL,  { "ScanLowOrEqual"  , 8, &DeviceFloppy::NotImplemented } },
			{ CMD::SCAN_HIGH_OR_EQUAL, { "ScanHighOrEqual" , 8, &DeviceFloppy::NotImplemented } }
		};

	};
}
