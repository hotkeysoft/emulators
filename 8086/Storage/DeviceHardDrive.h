#pragma once

#include "../CPU/PortConnector.h"
#include "../Serializable.h"
#include <vector>
#include <deque>

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace hdd
{
	struct Geometry
	{
		const char* name;
		BYTE head;
		WORD cyl;
		BYTE sect;

		uint32_t GetImageSize() const { return 512 * head * cyl * sect; }
		uint32_t CHS2A(WORD c, BYTE h, BYTE s) const { return 512 * ((c * head + h) * sect + s); }
	};

	class HardDisk
	{
	public:
		~HardDisk() { Clear(); }
		void Clear()
		{
			type = 0;
			path.clear();
			loaded = false;

			if (data)
			{
				fclose(data);
				data = nullptr;
			}
		}

		BYTE type;
		std::filesystem::path path;
		bool loaded = false;
		Geometry geometry;
		FILE* data = nullptr;
	};

	class DeviceHardDrive : public PortConnector, public emul::Serializable
	{

	public:
		DeviceHardDrive(WORD baseAddress, size_t clockSpeedHz);
		virtual ~DeviceHardDrive();

		DeviceHardDrive() = delete;
		DeviceHardDrive(const DeviceHardDrive&) = delete;
		DeviceHardDrive& operator=(const DeviceHardDrive&) = delete;
		DeviceHardDrive(DeviceHardDrive&&) = delete;
		DeviceHardDrive& operator=(DeviceHardDrive&&) = delete;

		virtual void Init();
		virtual void Reset();

		virtual void Tick();

		void CommandExecutionDone();

		bool IsActive(BYTE drive) { return m_commandBusy && (m_currDrive == drive); }

		bool LoadDiskImage(BYTE drive, BYTE type, const char* path);

		const HardDisk& GetImageInfo(BYTE drive) { assert(drive < 2); return m_images[drive]; }

		size_t DelayToTicks(size_t delayMS);

		BYTE ReadDataFIFO();
		void WriteDataFIFO(BYTE value);

		BYTE ReadStatus();
		void WriteControllerReset(BYTE value);

		BYTE ReadOptionJumpers();
		void WriteControllerSelectPulse(BYTE);

		void WriteMaskRegister(BYTE value);

		bool IsInterruptPending() const { return m_irqEnabled && m_interruptPending; }

		bool IsDMAPending() const { return m_dmaEnabled && m_dmaPending; }
		void DMAAcknowledge();
		void DMATerminalCount();

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

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

			SEEK_EXEC,
			INIT,
			INIT_PARAM_WAIT,

			RW_DONE,

			DMA_WAIT,
			DMA_ACK,

			NDMA_WAIT,

			RESULT_WAIT,
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
		void SeekTo();
		bool UpdateCurrPos();
		void PushStatus();
		void InitDrive2();

		// HDC Commands
		typedef STATE(DeviceHardDrive::* ExecFunc)();
		STATE NotImplemented();
		STATE TestDriveReady();
		STATE SenseStatus();
		STATE Diagnostic();
		STATE Recalibrate();
		STATE InitDrive();
		STATE WriteDataBuffer();
		STATE ReadSectors();
		STATE WriteSectors();

		bool m_dmaEnabled = false;
		bool m_dmaPending = false;
		virtual void SetDMAPending() { m_dmaPending = true; }

		bool m_irqEnabled = false;
		bool m_interruptPending = false;
		virtual void SetInterruptPending() { m_interruptPending = true; }
		void ClearInterrupt() { m_interruptPending = false; }

		// CommandBlock
		struct CommandBlock
		{
			BYTE drive = 0; // 0-1
			BYTE head = 0;
			WORD cylinder = 0;
			BYTE sector = 0;
			union
			{
				BYTE blockCount = 0;
				BYTE interleave;
			};
			bool noRetries = false;
			bool eccRetry = false;

			BYTE stepCode = 0;

		} m_commandBlock;
		void ReadCommandBlock();

		// Status
		enum HWSTATUS
		{
			HWS_REQ  = 0x01, // 1 = Ready for data to be transferred
			HWS_DIO  = 0x02, // 1 = Controller->CPU, 0 = CPU->Controller
			HWS_CD   = 0x04, // 1 = Data byte, 0 = command/status
			HWS_BUSY = 0x08, // 1 = Device Busy
			HWS_DRQ  = 0x10, // 1 = Ready for DMA transfer
			HWS_IRQ  = 0x20 // 1 = Interrupt pending
		};

		enum HDCERROR
		{
			ERR_OK = 0b000000, // Not an error

			// Errors type 0
			ERR_NO_INDEX = 0b000001,
			ERR_NO_SEEK = 0b000010,
			ERR_WRITE_FAULT = 0b000011,
			ERR_NOT_READY = 0b000100,
			ERR_NO_TRACK0 = 0b000110,
			ERR_SEEKING = 0b001000, // Drive is still seeking (Test Drive Ready command)

			// Errors type 1
			ERR_ID_READ = 0b010000,
			ERR_DATA = 0b010001,
			ERR_ADDRESS = 0b010010,
			ERR_SECTOR = 0b010100,
			ERR_SEEK = 0b010101,
			ERR_CORRECTABLE = 0b011000,
			ERR_BAD_TRACK = 0b011001,

			// Errors type 2
			ERR_INVALID_CMD = 0b100000,
			ERR_ILLEGAL_ADDR = 0b100001,

			// Errors type 3
			ERR_DIAG_RAM = 0b110000,
			ERR_DIAG_CHKSUM = 0b110001,
			ERR_DIAG_ECC = 0b110010
		};

		// Sense Bytes
		struct Sense
		{
			// set if previous command required a disk address
			bool addressValid = false;

			HDCERROR error = ERR_OK;

			BYTE drive = 0; // 0-1
			BYTE head = 0;
			WORD cylinder = 0;
			BYTE sector = 0;
		} m_sense;
		void SetLastState(HDCERROR state, bool addressValid = false);

		BYTE m_currDrive = 0; // 0-1
		WORD m_currCylinder = 0;
		BYTE m_currSector = 0;
		BYTE m_maxSector = 0;
		BYTE m_currHead = 0;

		WORD m_stepRate = 3000; // Step Rate Time, time between cylinders (us)

		bool m_commandBusy = false;
		bool m_commandError = false;
		bool m_dataRegisterReady = true;

		enum class ControlData { CONTROL = 0, DATA = 1 };
		ControlData m_controlData = ControlData::CONTROL;

		enum class DataDirection { CPU2HDC = 0, HDC2CPU = 1 };
		DataDirection m_dataInputOutput = DataDirection::CPU2HDC;

		// Command/Response/Parameters FIFO
		void Push(BYTE value) { m_fifo.push_back(value); }
		BYTE Pop() { BYTE ret = m_fifo.front(); m_fifo.pop_front(); return ret; }
		std::deque<BYTE> m_fifo;

		enum CMD
		{
			TEST_DRIVE = 0,
			RECALIBRATE = 1,
			SENSE = 3,
			FORMAT_DRIVE = 4,
			READ_VERIFY = 5,
			FORMAT_TRACK = 6,
			FORMAT_BAD_TRACK = 7,
			READ = 8,
			WRITE = 10,
			SEEK = 11,
			INIT_DRIVE = 12,
			READ_ECC_BURST_LEN = 13,
			READ_DATA_BUFFER = 14,
			WRITE_DATA_BUFFER = 15,

			RAM_DIAGNOSTIC = 0xE0,
			DRIVE_DIAGNOSTIC = 0xE3,
			CTRL_DIAGNOSTIC = 0xE4,

			READ_LONG = 0xE5,
			WRITE_LONG = 0xE6,
		};

		struct Command
		{
			const char* name;
			size_t paramCount;
			ExecFunc func;
		};
		const Command* m_currCommand = nullptr;
		BYTE m_currcommandID = 0;

		typedef std::map<CMD, Command> CommandMap;
		const CommandMap m_commandMap = {
			{ CMD::TEST_DRIVE,        { "Test Drive",            5,  &DeviceHardDrive::TestDriveReady } },
			{ CMD::RECALIBRATE,       { "Recalibrate",           5,  &DeviceHardDrive::Recalibrate } },
			{ CMD::SENSE,             { "Sense",                 5,  &DeviceHardDrive::SenseStatus } },
			{ CMD::FORMAT_DRIVE,      { "Format Drive",          5,  &DeviceHardDrive::NotImplemented } },
			{ CMD::READ_VERIFY,       { "Read Verify",           5,  &DeviceHardDrive::NotImplemented } },
			{ CMD::FORMAT_TRACK,      { "Format Track",          5,  &DeviceHardDrive::NotImplemented } },
			{ CMD::FORMAT_BAD_TRACK,  { "Format Bad Track",      5,  &DeviceHardDrive::NotImplemented } },
			{ CMD::READ,              { "Read",                  5,  &DeviceHardDrive::ReadSectors } },
			{ CMD::WRITE,             { "Write",                 5,  &DeviceHardDrive::WriteSectors } },
			{ CMD::SEEK,              { "Seek",                  5,  &DeviceHardDrive::NotImplemented } },
			{ CMD::INIT_DRIVE,        { "Init Drive",            5,  &DeviceHardDrive::InitDrive } },
			{ CMD::READ_ECC_BURST_LEN,{ "Read ECC Burst Length", 5,  &DeviceHardDrive::NotImplemented } },
			{ CMD::READ_DATA_BUFFER,  { "Read Data Buffer",      5,  &DeviceHardDrive::NotImplemented } },
			{ CMD::WRITE_DATA_BUFFER, { "Write Data Buffer",     5,  &DeviceHardDrive::WriteDataBuffer } },
			{ CMD::RAM_DIAGNOSTIC,    { "RAM Diagnostic",        5,  &DeviceHardDrive::Diagnostic } },
			{ CMD::DRIVE_DIAGNOSTIC,  { "Drive Diagnostic",      5,  &DeviceHardDrive::Diagnostic } },
			{ CMD::CTRL_DIAGNOSTIC,   { "Controller Diagnostic", 5,  &DeviceHardDrive::Diagnostic } },
			{ CMD::READ_LONG,         { "Read Long",             5,  &DeviceHardDrive::NotImplemented } },
			{ CMD::WRITE_LONG,        { "Write Long",            5,  &DeviceHardDrive::NotImplemented } }
		};

		typedef std::map<BYTE, Geometry> Geometries;
		const Geometries m_geometries = {
			// IBM/XEBEC BIOS
			//{ 1, { "Type 1 (10MB)",  4, 306, 17 } },
			//{ 16,{ "Type 16 (20MB)", 4, 612, 17 } },
			//{ 2, { "Type 2 (20MB)",  4, 615, 17 } },
			//{ 13,{ "Type 13 (20MB)", 8, 306, 17 } },

			// WD1002S-WX2 BIOS
			{ 0, { "Type 0 (20MB)", 4, 612, 17 } },
			{ 1, { "Type 1 (10MB)", 2, 612, 17 } },
			{ 2, { "Type 2 (20MB)", 4, 612, 17 } },
			{ 3, { "Type 3 (10MB)", 4, 306, 17 } },

			// Custom
			// 
			// Dip switches return "0" for both drives.
			// The first drive table entry in ROM is 306/4/17 or 612/4/17 
			// depending on BIOS used. We can use images with different 
			// cylinder count, but the head and sector count must stay the 
			// same for it to work correctly.
			//
			// TODO: Ideally the image geometry would be patched in the BIOS.
			// This would allow using the maximum number of heads allowed.
			{ 20,{ "Type 20 (20MB)", 4, 615, 17 } },
			{ 33,{ "Type 33 (33MB)", 4, 1000, 17 } },
		};

		HardDisk m_images[2];

		BYTE m_sectorBuffer[512];
	};
}
