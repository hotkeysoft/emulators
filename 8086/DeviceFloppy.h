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

			RQM_DELAY,

			PARAM_WAIT,

			CMD_WAIT,
			CMD_READ,
			CMD_ERROR,
			CMD_EXEC,
			CMD_EXEC_DELAY,
			CMD_EXEC_DONE,

			RESULT_WAIT,
		} m_state, m_nextState;

		// State machine processing
		void RQMDelay(STATE nextState);
		void ReadCommand();
		void ExecuteCommand();

		// FDC Commands
		typedef size_t(DeviceFloppy::* ExecFunc)();
		size_t NotImplemented();
		size_t SenseInterrupt();
		size_t Recalibrate();
		size_t Seek();
		size_t SenseDriveStatus();
		size_t Specify();
		size_t ReadData();

		bool m_interruptPending;
		void SetInterruptPending() { m_interruptPending = true; }

		// Status
		enum MSR
		{
			RQM  = 0x80, // 1 = Data Register Ready
			DIO  = 0x40, // 1 = Controller->CPU, 0 = CPU->Controller
			NDMA = 0x20, // 1 = NON-DMA Mode, 0 = DMA Mode
			BUSY = 0x10, // 1 = Device Busy
			ACTD = 0x08, // 1 = Drive 3 Seeking
			ACTC = 0x04, // 1 = Drive 2 Seeking
			ACTB = 0x02, // 1 = Drive 1 Seeking 
			ACTA = 0x01, // 1 = Drive 0 Seeking
		};

		enum ST0
		{
			IC1 = 0x80, // 00 = Normal Termination, 01 = Abnormal Termination
			IC0 = 0x40, // 10 = Invalid Command, 00 = Drive not ready
			SE  = 0x20, // 1 = Seek End
			EC  = 0x10, // 1 = Equipment Check (fault, no track 0)
			NR  = 0x08, // 1 = Not Ready
			HD  = 0x04, // Head Address
			US1 = 0x02, // Drive Number at interrupt
			US0 = 0x01, // Drive Number at interrupt
		};

		enum ST3
		{
			ESIG = 0x80, // 1 = Error
			WPDR = 0x40, // 1 = Write Protection
			RDY =  0x20, // 1 = Ready
			TRK0 = 0x10, // Above track 0
			DSDR = 0x08, // Double sided drive
			HDDR = 0x04, // Active head
			DS1 =  0x02, // Drive Number
			DS0 =  0x01, // Drive Number
		};
		BYTE m_st0; // Status flag
		BYTE m_st3; // Status flag
		BYTE m_pcn; // Present Cylinder Number

		BYTE m_srt; // Step Rate Time, time between cylinders (ms)
		BYTE m_hlt; // Head Load Time, time to wait between activating head and before read (ms)
		BYTE m_hut; // Head Unload Time, time to wait before deactivating the head (ms)
		bool m_nonDMA;

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

		struct Command
		{
			const char* name;
			size_t paramCount;
			ExecFunc func;
			bool interrupt;
		};
		const Command* m_currCommand;
		BYTE m_currcommandID;

		typedef std::map<CMD, Command> CommandMap;
		const CommandMap m_commandMap = {
			{ CMD::READ_TRACK,         { "ReadTrack"       , 8, &DeviceFloppy::NotImplemented,   false } },
			{ CMD::SPECIFY,            { "Specify"         , 2, &DeviceFloppy::Specify,          false } },
			{ CMD::SENSE_DRIVE_STATUS, { "SenseDriveStatus", 1, &DeviceFloppy::SenseDriveStatus, false } },
			{ CMD::WRITE_DATA,         { "WriteData"       , 8, &DeviceFloppy::NotImplemented,   false } },
			{ CMD::READ_DATA,          { "ReadData"        , 8, &DeviceFloppy::ReadData,         true  } },
			{ CMD::RECALIBRATE,        { "Recalibrate"     , 1, &DeviceFloppy::Recalibrate,      true  } },
			{ CMD::SENSE_INT_STATUS,   { "SenseInterrupt"  , 0, &DeviceFloppy::SenseInterrupt,   false } },
			{ CMD::WRITE_DELETED_DATA, { "WriteDeletedData", 8, &DeviceFloppy::NotImplemented,   false } },
			{ CMD::READ_ID,            { "ReadID"          , 1, &DeviceFloppy::NotImplemented,   false } },
			{ CMD::READ_DELETED_DATA,  { "ReadDeletedData" , 8, &DeviceFloppy::NotImplemented,   false } },
			{ CMD::FORMAT_TRACK,       { "FormatTrack"     , 5, &DeviceFloppy::NotImplemented,   false } },
			{ CMD::SEEK,               { "Seek"            , 2, &DeviceFloppy::Seek,             true  } },
			{ CMD::SCAN_EQUAL,         { "ScanEqual"       , 8, &DeviceFloppy::NotImplemented,   false } },
			{ CMD::SCAN_LOW_OR_EQUAL,  { "ScanLowOrEqual"  , 8, &DeviceFloppy::NotImplemented,   false } },
			{ CMD::SCAN_HIGH_OR_EQUAL, { "ScanHighOrEqual" , 8, &DeviceFloppy::NotImplemented,   false } }
		};

	};
}
