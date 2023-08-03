#pragma once

#include <CPU/CPUCommon.h>
#include "DeviceFloppy.h"
#include <vector>
#include <deque>

using emul::WORD;
using emul::BYTE;

namespace fdc
{
	namespace dsk
	{
#pragma pack(push, 1)
		struct DiscInfo
		{
			char id[34] = { 0 };
			char creator[14] = { 0 };
			BYTE tracks = 0;
			BYTE sides = 0;
			WORD trackSize = 0;  // Unused in EXT format
			BYTE trackSizes[204]; // EXT format only

			void Clear() { memset(this, 0, sizeof(DiscInfo)); }
			bool IsDSK() const { return (memcmp(id, "MV - CPC", 8) == 0) && tracks && sides && trackSize; }
			bool IsExt() const { return (memcmp(id, "EXTENDED", 8) == 0) && tracks && sides && trackSizes[0]; }

			WORD GetTracksize(BYTE track) const { return trackSizes[track] * 256; }
		};

		struct SectorInfo
		{
			BYTE track = 0;      // C
			BYTE side = 0;       // H
			BYTE sectorID = 0;   // R
			BYTE sectorSize = 0; // N
			BYTE FDCStatus1 = 0; // ST1
			BYTE FDCStatus2 = 0; // ST2
			WORD dataLength = 0; // Actual data length in bytes, Extended format only
		};

		struct TrackInfo
		{
			char id[12] = { 0 };
			BYTE _padding1[4];
			BYTE track = 0;
			BYTE side = 0;
			BYTE _padding2[2];
			BYTE sectorSize = 0;
			BYTE sectorCount = 0;
			BYTE gapLength = 0;
			BYTE _padding3 = 0;
			SectorInfo sectorInfo[29] = { 0 };

			void Clear(){ memset(this, 0, sizeof(TrackInfo)); }
			bool IsValid() const { return (memcmp(id, "Track-Info", 10) == 0) && sectorSize && sectorCount; }
		};

#pragma pack(pop)

		using RawData = std::vector<BYTE>;
		struct TrackData
		{
			BYTE sectorOffset = (BYTE)-1;
			RawData data;

			bool isValid() const { return data.size() && isSectorOffsetSet(); }
			bool isSectorOffsetSet() const { return sectorOffset != (BYTE)-1; }
			void Clear() { data.clear(); sectorOffset = (BYTE)-1; }
		};
	}

	class DeviceFloppyCPC : public DeviceFloppy
	{
	public:
		DeviceFloppyCPC(WORD baseAddress, size_t clockSpeedHz);

		DeviceFloppyCPC() = delete;
		DeviceFloppyCPC(const DeviceFloppyCPC&) = delete;
		DeviceFloppyCPC& operator=(const DeviceFloppyCPC&) = delete;
		DeviceFloppyCPC(DeviceFloppyCPC&&) = delete;
		DeviceFloppyCPC& operator=(DeviceFloppyCPC&&) = delete;

		virtual void Init() override;
		void Reset();

		virtual bool LoadDiskImage(BYTE drive, const char* path) override;

		virtual bool IsActive(BYTE drive) override { return m_motor; };

	protected:
		BYTE ReadFloppyController();
		void WriteFloppyController(BYTE value);
		void WriteFloppyMotorControl(BYTE value);

		bool LoadFloppy(BYTE drive, const FloppyDisk& floppy);

		bool m_motor = false;

		dsk::DiscInfo  ReadDiscInfoBlock(FILE* f);
		dsk::TrackInfo ReadTrackInfoBlock(FILE* f);
		dsk::TrackData ReadTrackData(const dsk::TrackInfo& trackInfo, FILE* f);

	};
}
