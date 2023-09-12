#pragma once

#include <CPU/PortConnector.h>
#include <Serializable.h>
#include <vector>
#include <deque>

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace fdd
{
	enum class DiskFormat { Mac400K = 0x02, Mac800K = 0x22 };
	using DATA = std::vector<BYTE>;

	constexpr int SECTOR_TAG_SIZE = 12;
	constexpr int SECTOR_DATA_SIZE = 512; 

#pragma pack(push, 1)
	struct RawSectorTag
	{
		DWORD fileNumber = 0;
		WORD flags = 0;
		WORD blockNumber = 0;
		DWORD timestamp = 0;
	};
#pragma pack(pop)
	static_assert(sizeof(RawSectorTag) == SECTOR_TAG_SIZE);

	struct RawSectorData
	{
		RawSectorTag tag;
		BYTE data[SECTOR_DATA_SIZE] = { 0 };
	};
	using RawSectors = std::vector<RawSectorData>;

	class FloppyDisk : public Logger
	{
	public:
		FloppyDisk() : Logger("disk") {}

		void Clear()
		{
			path.clear();
			loaded = false;
			m_tracks.clear();
		}

		void Init(DiskFormat format);
		bool Load(std::filesystem::path);

		std::filesystem::path path;
		bool loaded = false;
		

		const RawSectors& GetTrack(int track);
		int GetTrackCount() const { return (int)m_tracks.size(); }

		using Interleave = std::vector<int>;
		const Interleave& GetInterleave(int track) const;
		int GetSectorCount(int track) const;

	protected:
		using Tracks = std::vector<RawSectors>;
		Tracks m_tracks;

		static const Interleave s_speedGroups[5];	
	};

	enum class StepDirection { OUTER = -1, INNER = 1 };

	class DeviceFloppy : public emul::Serializable, public Logger
	{

	public:
		DeviceFloppy(uint32_t clockSpeedHz, const char* id = "fdd");
		virtual ~DeviceFloppy() {};

		DeviceFloppy() = delete;
		DeviceFloppy(const DeviceFloppy&) = delete;
		DeviceFloppy& operator=(const DeviceFloppy&) = delete;
		DeviceFloppy(DeviceFloppy&&) = delete;
		DeviceFloppy& operator=(DeviceFloppy&&) = delete;

		virtual void EnableLog(Logger::SEVERITY minSev = Logger::LOG_INFO) override;

		virtual void Init(bool connected = true) { m_connected = connected;  Reset(); };
		virtual void Reset();

		virtual void Tick();

		virtual bool ClearDiskImage() { m_diskLoaded = false; m_disk.Clear(); return true; };
		virtual bool LoadDiskImage(const char* path);
		virtual bool SaveDiskImage(const char* path) { return false; }

		//const FloppyDisk& GetImageInfo(BYTE drive) { assert(drive < 4); return m_images[drive]; }

		virtual bool IsActive() const { return IsDiskLoaded() && IsMotorEnabled(); }

		// ==========
		// Drive
		// ==========
		bool IsConnected() const { return m_connected; }

		// ==========
		// Disk
		// ==========
		bool IsDiskLoaded() const { return m_diskLoaded; }
		bool IsDiskChanged() const { return m_diskChanged; }
		void ClearDiskChanged() { m_diskChanged = false; }

		// ==========
		// Rotation
		// ==========

		// Starts/stop the motor spinning the disk
		// (disk doesn't need to be present)
		void EnableMotor(bool enable);
		bool IsMotorEnabled() const { return m_motorEnabled; }

		// Sets the speed of the motor (in RPM)
		void SetMotorSpeed(WORD rpm, bool force = false);
		WORD GetMotorSpeed() const { return m_motorSpeed; }

		// Pulses on and off 60 times per rotation
		bool GetMotorPulse() const { return m_motorPulse; }

		// ==========
		// Head
		// ==========

		// Sets time it takes for the head to step one track (in milliseconds)
		void SetStepDelay(WORD millis);
		WORD GetStepDelay() const { return m_stepDelay; }

		// Set head direction (OUTER = towards track 0)
		StepDirection GetStepDirection() const { return m_stepDirection; }
		void SetStepDirection(StepDirection dir);

		// Step one track in the current StepDirection
		// (disk doesn't need to be present)
		void Step(); 

		// True if the head is currently moving between tracks
		bool IsSeeking() const { return m_isSeeking; }

		// True if the head is currently stopped at track zero
		bool IsTrack0() const { return IsActive() && !m_isSeeking && (m_currTrack == 0) ; }

		// Tracks are numbered [0..TrackCount - 1]
		WORD GetTrackCount() const { return m_trackCount; }
		void SetTrackCount(WORD tracks); // Set number of tracks

		WORD GetCurrentTrack() const { return m_currTrack; }

		WORD GetCurrentSector() const { return m_currSectorData->sector; }

		WORD GetHeadCount() const { return m_headCount; }
		void SetHeadCount(WORD heads); // Select single or double sided (1 or 2)

		WORD GetCurrentHead() const { return m_currHead; }
		void SelectHead(WORD head); // Heads are numbered [0..HeadCount-1]

		bool IsCalibrating() const { return m_isCalibrating; }
		// Launch drive calibration (seek to inner and back to track zero)
		void Calibrate();

		// ==========
		// Data
		// ==========
		BYTE ReadByte();

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		const uint32_t m_clockSpeed;
		bool m_connected = false;

		// Add with carry, returns carry out
		bool ADC(BYTE& dest, BYTE src, bool carryIn);
		// Rotate left, returns rotated bit out
		bool ROL(BYTE& dest);

		uint32_t DelayToTicks(uint32_t millis) { return millis * m_clockSpeed / 1000; }

		// Disk
		bool m_diskLoaded = false;
		bool m_diskChanged = false;

		// Rotation
		static constexpr WORD MIN_RPM = 60;
		static constexpr WORD MAX_RPM = 1000;
		static constexpr WORD DEFAULT_RPM = 360;
		WORD m_motorSpeed = DEFAULT_RPM;
		bool m_motorEnabled = false;

		void ResetPulseCounter() { m_motorPulseCounter = m_ticksPerRotation; }
		uint32_t m_ticksPerRotation = UINT32_MAX;
		uint32_t m_motorPulseCounter = UINT32_MAX;
		bool m_motorPulse = false; // Ticks on/off 60 times per revolution

		// Head
		static constexpr WORD MIN_STEP_MS = 1;
		static constexpr WORD MAX_STEP_MS = 100;
		static constexpr WORD MIN_HEADS = 1;
		static constexpr WORD MAX_HEADS = 2;
		static constexpr WORD MIN_TRACKS = 10;
		static constexpr WORD MAX_TRACKS = 100;
		static constexpr WORD DEFAULT_STEP_MS = 10;
		static constexpr WORD DEFAULT_HEAD_COUNT = 1;
		static constexpr WORD DEFAULT_TRACK_COUNT = 40;

		WORD m_stepDelay = DEFAULT_STEP_MS;
		uint32_t m_ticksPerTrack = UINT32_MAX;
		uint32_t m_seekCounter = UINT32_MAX;
		void ResetSeekCounter() { m_seekCounter = m_ticksPerTrack; }

		bool m_isCalibrating = false;
		bool m_isSeeking = false;
		StepDirection m_stepDirection = StepDirection::INNER;

		WORD m_trackCount = DEFAULT_TRACK_COUNT;
		WORD m_currTrack = 0;
		void NewTrack(WORD track);

		// The index in the array of sectors for a track. This will be different
		// from the actual "data" sector id due to interleaving
		WORD m_currSectorIndex = 0;
		void NextSector();

		WORD m_headCount = DEFAULT_HEAD_COUNT;
		WORD m_currHead = 0;

		// Data
		DATA BuildHeader(BYTE track, BYTE sector, BYTE side, DiskFormat format);
		DATA BuildData(BYTE sector, const RawSectorData& rawData);
	
		static constexpr int ENCODED_SECTOR_SIZE = 750;
		struct EncodedSector
		{
			int sector = 0;
			DATA data;
		};
		using EncodedTrack = std::vector<EncodedSector>;

		using EncodedTracks = std::vector<EncodedTrack>;
		EncodedTracks m_trackData;
		EncodedSector* m_currSectorData = nullptr;
		int m_currSectorPos = 0;	
		
		void ResetChecksum() { m_checksumA = 0; m_checksumB = 0; m_checksumC = 0; }
		DATA EncodeDataBlock(const BYTE data[], int size);
		void Encode3To4(BYTE encodedOut[4], BYTE byteA, BYTE byteB, BYTE byteC, bool lastBlock = false);

		// Running checksum for data block
		BYTE m_checksumA = 0;
		BYTE m_checksumB = 0;
		BYTE m_checksumC = 0;
	
		// Disk
		FloppyDisk m_disk;
	};
}
