#include "stdafx.h"

#include "DeviceFloppyCPC464.h"

using hscommon::fileUtil::File;

namespace fdc
{
	const emul::BitMaskW StatusRegisterMask("0xxxxxx0");
	const emul::BitMaskW DataRegisterMask("0xxxxxx1");
	const emul::BitMaskW MotorControlMask("0xxxxxxx");

	static const char* DISK_ID = "CPC_DSK";

	static_assert(sizeof(dsk::DiscInfo) == 256);
	static_assert(sizeof(dsk::SectorInfo) == 8);
	static_assert(sizeof(dsk::TrackInfo) == 256);

	DeviceFloppyCPC464::DeviceFloppyCPC464(WORD baseAddress, size_t clockSpeedHz) :
		DeviceFloppy(baseAddress, clockSpeedHz),
		Logger("floppyCPC464")
	{
	}

	void DeviceFloppyCPC464::Reset()
	{
		DeviceFloppy::Reset();
		m_motor = false;
	}

	void DeviceFloppyCPC464::Init()
	{
		//MAIN_STATUS_REGISTER = 0x3F4, // read-only
		Connect("xxxxx0x1", static_cast<PortConnector::INFunction>(&DeviceFloppyCPC464::ReadFloppyController));
		Connect("xxxxx0x1", static_cast<PortConnector::OUTFunction>(&DeviceFloppyCPC464::WriteFloppyController));

		Connect("xxxxx0x0", static_cast<PortConnector::OUTFunction>(&DeviceFloppyCPC464::WriteFloppyMotorControl));

		SetDiskChanged();
	}

	BYTE DeviceFloppyCPC464::ReadFloppyController()
	{
		const WORD port = GetCurrentPort();
		LogPrintf(LOG_TRACE, "ReadFDC, port=%04X", port);

		if (StatusRegisterMask.IsMatch(port))
			return ReadMainStatusReg();
		else if (DataRegisterMask.IsMatch(port))
			return ReadDataFIFO();
		else
			return 0xFF;
	}
	void DeviceFloppyCPC464::WriteFloppyController(BYTE value)
	{
		const WORD port = GetCurrentPort();
		LogPrintf(LOG_TRACE, "WriteFDC, port = %04X, value=%02X", GetCurrentPort(), value);

		if (DataRegisterMask.IsMatch(port))
			return WriteDataFIFO(value);
	}

	void DeviceFloppyCPC464::WriteFloppyMotorControl(BYTE value)
	{
		const WORD port = GetCurrentPort();
		LogPrintf(LOG_TRACE, "WriteMotorControl, port = %04X, value=%02X", GetCurrentPort(), value);

		if (MotorControlMask.IsMatch(port))
		{
			m_motor = (value != 0);
		}
	}

	bool DeviceFloppyCPC464::LoadFloppy(BYTE drive, const FloppyDisk& floppy)
	{
		if (drive > 3)
		{
			LogPrintf(LOG_ERROR, "LoadFloppy: invalid drive number %d", drive);
			return false;
		}

		LogPrintf(LOG_INFO, "LoadFloppy: loading floppy image in drive %d", drive);

		// Sanity check
		if (!floppy.geometry.IsSet() || (floppy.data.size() != floppy.geometry.GetImageSize()))
		{
			LogPrintf(LOG_ERROR, "LoadFloppy: Invalid floppy image");
			return false;
		}

		SetDiskChanged();

		m_images[drive] = floppy;

		return true;
	}

	bool DeviceFloppyCPC464::LoadDiskImage(BYTE drive, const char* path)
	{
		if (drive > 3)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: invalid drive number %d", drive);
			return false;
		}

		LogPrintf(LOG_INFO, "LoadDiskImage: loading %s in drive %d", path, drive);

		File f(path, "rb");
		if (!f)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: error opening binary file");
			return false;
		}

		dsk::DiscInfo discInfo = ReadDiscInfoBlock(f);
		if (!discInfo.IsValid())
		{
			// Fallback to raw file loader
			f.Close();
			LogPrintf(LOG_WARNING, "File doesn't seem to be in dsk format, trying raw loader");

			return DeviceFloppy::LoadDiskImage(drive, path);
		}

		LogPrintf(LOG_INFO, "id        : %s", discInfo.id);
		LogPrintf(LOG_INFO, "creator   : %s", discInfo.creator);
		LogPrintf(LOG_INFO, "tracks    : %d", discInfo.tracks);
		LogPrintf(LOG_INFO, "sides     : %d", discInfo.sides);
		LogPrintf(LOG_INFO, "track size: %d", discInfo.trackSize);

		Geometry geometry;
		geometry.name = DISK_ID;
		geometry.cyl = discInfo.tracks;
		geometry.head = discInfo.sides;
		geometry.sect = 0;
		geometry.sectOffset = 0xC0; // TODO: Only 'data' format supported at the moment, see below

		// Standard CPC disc formats ( from https://www.cpcwiki.eu/index.php/Disc_format )
		//
		//	Data:	180k single-sided
		//			40 tracks of nine 512-byte sectors each.
		//			64 directory entries.
		//			Capacity: 178k
		//
		//	System:	180k single-sided
		//			40 tracks of nine 512-byte sectors each.
		//			Sectors: 0x41-0x49
		//			64 directory entries.
		//			Two reserved tracks containing CP/M BIOS.
		//
		//	IBM:	160k single-sided
		//			40 tracks of eight 512-byte sectors each.
		//			Sectors: 0x01-0x08
		//			Provided for compatibility with CP/M-86 (and DR-DOS 1.0).

		// Sanity checks
		if (discInfo.tracks > 100 || discInfo.sides > 2)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: invalid geometry");
			return false;
		}

		FloppyDisk floppy;
		floppy.data.reserve(geometry.GetImageSize());

		// Read tracks
		for (int c = 0; c < discInfo.tracks; ++c)
		{
			for (int h = 0; h < discInfo.sides; ++h)
			{
				dsk::TrackInfo trackInfo = ReadTrackInfoBlock(f);
				if (!trackInfo.IsValid())
				{
					// Fallback to raw file loader
					LogPrintf(LOG_WARNING, "Invalid Track Info for track[%d,%d]", c, h);
					return false;
				}

				// Sanity checks
				if (geometry.sect && (geometry.sect != trackInfo.sectorCount))
				{
					LogPrintf(LOG_ERROR, "LoadDiskImage: Not supported: track[%d,%d] has a different sector count %d != %d", c, h, geometry.sect, trackInfo.sectorCount);
					return false;
				}

				geometry.sect = trackInfo.sectorCount;
				if ((trackInfo.sectorSize != 2) || (trackInfo.track != c) || (trackInfo.side != h))
				{
					LogPrintf(LOG_ERROR, "LoadDiskImage: track[%d,%d] has invalid info ", c, h);
					return false;
				}

				dsk::TrackData trackData = ReadTrackData(trackInfo, f);
				if (trackData.empty())
				{
					LogPrintf(LOG_ERROR, "LoadDiskImage: track[%d,%d] has invalid data", c, h);
					return false;
				}

				floppy.data.insert(floppy.data.end(), trackData.begin(), trackData.end());
			}
		}

		floppy.geometry = geometry;
		floppy.loaded = true;
		floppy.path = path;

		if (floppy.data.size() != geometry.GetImageSize())
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: floppy size != expected (%d != %d)", floppy.data.size(), geometry.GetImageSize());
			return false;
		}

		LoadFloppy(drive, floppy);

		SaveDiskImage(0, "dump/disk.raw");

		return true;
	}

	dsk::DiscInfo DeviceFloppyCPC464::ReadDiscInfoBlock(FILE* f)
	{
		dsk::DiscInfo info;
		if (fread(&info, sizeof(info), 1, f) != 1)
		{
			LogPrintf(LOG_WARNING, "Error reading disc information block");
			info.Clear();
		}

		return info;
	}

	dsk::TrackInfo DeviceFloppyCPC464::ReadTrackInfoBlock(FILE* f)
	{
		dsk::TrackInfo info;
		if (fread(&info, sizeof(info), 1, f) != 1)
		{
			LogPrintf(LOG_WARNING, "Error reading track info");
			info.Clear();
		}

		return info;
	}

	dsk::TrackData DeviceFloppyCPC464::ReadTrackData(const dsk::TrackInfo& trackInfo, FILE* f)
	{
		dsk::TrackData trackData;

		using SectorData = std::array<BYTE, 512>;

		std::map<BYTE, SectorData> sectors;
		SectorData buf;

		for (int s = 0; s < trackInfo.sectorCount; ++s)
		{
			const dsk::SectorInfo& info = trackInfo.sectorInfo[s];
			LogPrintf(LOG_INFO, "Reading data for chs=[%d,%d,%d]", info.track, info.side, info.sectorID);

			if (fread(&buf, 512, 1, f) != 1)
			{
				LogPrintf(LOG_WARNING, "Error reading track data");
				trackData.clear();
				break;
			}

			sectors[info.sectorID] = buf;
		}

		for (auto& sector : sectors)
		{
			LogPrintf(LOG_DEBUG, "Sector %x", sector.first);
			trackData.insert(trackData.end(), sector.second.begin(), sector.second.end());
		}

		const int expected = 512 * trackInfo.sectorCount;
		if (expected != trackData.size())
		{
			LogPrintf(LOG_WARNING, "Track Data size expected != actual (%d != %d)", expected, trackData.size());
			trackData.clear();
		}

		return trackData;
	}

}
