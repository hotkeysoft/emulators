#pragma once

#include "../CPU/PortConnector.h"
#include "../Serializable.h"
#include <vector>
#include <deque>

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace fdc
{
	struct Geometry
	{
		const char* name;
		BYTE head;
		BYTE cyl;
		BYTE sect;
		uint32_t GetImageSize() { return 512 * head * cyl * sect; }
		uint32_t CHS2A(BYTE c, BYTE h, BYTE s) { return 512 * ((c * head + h) * sect + s - 1); }
	};

	class FloppyDisk
	{
	public:
		void Clear()
		{
			path.clear();
			loaded = false;
			data.clear();
		}

		std::filesystem::path path;
		bool loaded = false;
		Geometry geometry;
		std::vector<BYTE> data;
	};

	class DeviceFloppy : public PortConnector, public emul::Serializable
	{
	protected:
		DeviceFloppy(WORD baseAddress, size_t clockSpeedHz);

	public:
		virtual ~DeviceFloppy() {};

		DeviceFloppy() = delete;
		DeviceFloppy(const DeviceFloppy&) = delete;
		DeviceFloppy& operator=(const DeviceFloppy&) = delete;
		DeviceFloppy(DeviceFloppy&&) = delete;
		DeviceFloppy& operator=(DeviceFloppy&&) = delete;

		virtual void Init();
		virtual void Reset();

		virtual void Tick();

		bool ClearDiskImage(BYTE drive);
		bool LoadDiskImage(BYTE drive, const char* path);
		bool SaveDiskImage(BYTE drive, const char* path);

		const FloppyDisk& GetImageInfo(BYTE drive) { assert(drive < 4); return m_images[drive]; }

		virtual bool IsActive(BYTE drive) = 0;

		size_t DelayToTicks(size_t delayMS);

		BYTE ReadMainStatusReg();

		BYTE ReadDataFIFO();
		void WriteDataFIFO(BYTE value);

		bool IsInterruptPending() const { return m_interruptPending; }

		bool IsDMAPending() const { return m_dmaPending; }
		void DMAAcknowledge();
		void DMATerminalCount();

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(json& from);

	protected:
		const WORD m_baseAddress;
		size_t m_clockSpeed;
		size_t m_currOpWait = 0;

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

			READ_START,
			READ_EXEC,

			WRITE_START,
			WRITE_EXEC,

			RW_DONE,

			NDMA_WAIT,
			DMA_WAIT,
			DMA_ACK,

			RESULT_WAIT,
			NOT_READY,
		};
		STATE m_state = STATE::CMD_WAIT;
		STATE m_nextState = STATE::CMD_WAIT;

		// State machine processing
		void RQMDelay(STATE nextState);
		void ReadCommand();
		void ExecuteCommand();
		void ReadSector();
		void RWSectorEnd();
		void WriteSector();
		bool UpdateCurrPos();

		// FDC Commands
		typedef STATE(DeviceFloppy::* ExecFunc)();
		STATE NotImplemented();
		STATE SenseInterrupt();
		STATE Recalibrate();
		STATE Seek();
		STATE SenseDriveStatus();
		STATE Specify();
		STATE ReadData();
		STATE ReadTrack();
		STATE WriteData();

		bool m_dmaPending = false;
		virtual void SetDMAPending() { m_dmaPending = true; }

		bool m_interruptPending = false;
		virtual void SetInterruptPending() { m_interruptPending = true; }
		void ClearInterrupt() { m_interruptPending = false; }

		// Status
		enum MSR
		{
			RQM  = 0x80, // 1 = Data Register Ready
			DIO  = 0x40, // 1 = Controller->CPU, 0 = CPU->Controller
			EXM  = 0x20, // 1 = During execution phase in non-DMA mode, 0 otherwise
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
		BYTE m_st0 = 0; // Status flag
		BYTE m_st3 = 0; // Status flag
		BYTE m_pcn = 0; // Present Cylinder Number (TODO: one per drive)
		
		BYTE m_currSector = 0;
		BYTE m_maxSector = 0;
		BYTE m_currHead = 0;
		bool m_multiTrack = false;

		BYTE m_srt = 0; // Step Rate Time, time between cylinders (ms)
		BYTE m_hlt = 0; // Head Load Time, time to wait between activating head and before read (ms)
		BYTE m_hut = 0; // Head Unload Time, time to wait before deactivating the head (ms)
		bool m_nonDMA = true;

		bool m_commandBusy = false;
		bool m_dataRegisterReady = true;
		bool m_executionPhase = false;
		bool m_driveActive[4] = { false, false, false, false }; // Only used for seek
		BYTE m_currDrive = 0;

		enum class DataDirection { FDC2CPU, CPU2FDC };
		DataDirection m_dataInputOutput = DataDirection::CPU2FDC;
		
		// Command/Response/Parameters FIFO
		void Push(BYTE value) { m_fifo.push_back(value); }
		BYTE Pop() { BYTE ret = m_fifo.front(); m_fifo.pop_front(); return ret; }
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
		const Command* m_currCommand = nullptr;
		BYTE m_currcommandID = 0;

		typedef std::map<CMD, Command> CommandMap;
		const CommandMap m_commandMap = {
			{ CMD::READ_TRACK,         { "ReadTrack"       , 8, &DeviceFloppy::ReadTrack,        true  } },
			{ CMD::SPECIFY,            { "Specify"         , 2, &DeviceFloppy::Specify,          false } },
			{ CMD::SENSE_DRIVE_STATUS, { "SenseDriveStatus", 1, &DeviceFloppy::SenseDriveStatus, false } },
			{ CMD::WRITE_DATA,         { "WriteData"       , 8, &DeviceFloppy::WriteData,        true  } },
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

		typedef std::map<uint32_t, Geometry> Geometries;
		const Geometries m_geometries = {
			{ 163840,  { "160KB",  1, 40, 8 } },
			{ 184320,  { "180KB",  1, 40, 9 } },
			{ 204800,  { "200KB",  1, 40, 10 } },

			{ 327680,  { "320KB",  2, 40, 8 } },
			{ 368640,  { "360KB",  2, 40, 9 } },
			{ 409600,  { "400KB",  2, 40, 10 } },

			{ 1228800, { "1.2MB",  2, 80, 15 } },

			{ 737280,  { "720KB",  2, 80, 9 } },
			{ 1474560, { "1.44MB", 2, 80, 18 } },
		};

		FloppyDisk m_images[4];
	};
}
