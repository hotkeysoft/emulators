#include "stdafx.h"

#include "DeviceFloppyCPC464.h"
#include <set>

using hscommon::fileUtil::File;

namespace fdc
{
	const emul::BitMaskW StatusRegisterMask("0xxxxxx0");
	const emul::BitMaskW DataRegisterMask("0xxxxxx1");
	const emul::BitMaskW MotorControlMask("0xxxxxxx");

	static const char* DISK_ID = "CPC-DSK";

	static_assert(sizeof(dsk::DiscInfo) == 256);
	static_assert(sizeof(dsk::SectorInfo) == 8);
	static_assert(sizeof(dsk::TrackInfo) == 256);

	DeviceFloppyCPC::DeviceFloppyCPC(WORD baseAddress, size_t clockSpeedHz) :
		DeviceFloppy(baseAddress, clockSpeedHz),
		Logger("floppyCPC")
	{
	}

	void DeviceFloppyCPC::Reset()
	{
		DeviceFloppy::Reset();
		m_motor = false;
	}

	void DeviceFloppyCPC::Init()
	{
		//MAIN_STATUS_REGISTER = 0x3F4, // read-only
		Connect("xxxxx0x1", static_cast<PortConnector::INFunction>(&DeviceFloppyCPC::ReadFloppyController), true);
		Connect("xxxxx0x1", static_cast<PortConnector::OUTFunction>(&DeviceFloppyCPC::WriteFloppyController), true);

		Connect("xxxxx0x0", static_cast<PortConnector::OUTFunction>(&DeviceFloppyCPC::WriteFloppyMotorControl), true);

		SetDiskChanged();
	}

	BYTE DeviceFloppyCPC::ReadFloppyController()
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
	void DeviceFloppyCPC::WriteFloppyController(BYTE value)
	{
		const WORD port = GetCurrentPort();
		LogPrintf(LOG_TRACE, "WriteFDC, port = %04X, value=%02X", GetCurrentPort(), value);

		if (DataRegisterMask.IsMatch(port))
			return WriteDataFIFO(value);
	}

	void DeviceFloppyCPC::WriteFloppyMotorControl(BYTE value)
	{
		const WORD port = GetCurrentPort();
		LogPrintf(LOG_TRACE, "WriteMotorControl, port = %04X, value=%02X", GetCurrentPort(), value);

		if (MotorControlMask.IsMatch(port))
		{
			m_motor = (value != 0);
		}
	}

	bool DeviceFloppyCPC::LoadFloppy(BYTE drive, const FloppyDisk& floppy)
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

	bool DeviceFloppyCPC::LoadDiskImage(BYTE drive, const char* path)
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

		bool extended = false;
		if (discInfo.IsDSK())
		{
			LogPrintf(LOG_INFO, "Detected DSK (base) signature");
			extended = false;
		}
		else if (discInfo.IsExt())
		{
			LogPrintf(LOG_INFO, "Detected DSK (extended) signature");
			extended = true;
		}
		else
		{
			// Fallback to raw file loader
			f.Close();
			LogPrintf(LOG_WARNING, "File doesn't seem to be in dsk format, trying raw loader");

			return DeviceFloppy::LoadDiskImage(drive, path);
		}

		LogPrintf(LOG_INFO, "id        : %.34s", discInfo.id);
		LogPrintf(LOG_INFO, "creator   : %.14s", discInfo.creator);
		LogPrintf(LOG_INFO, "tracks    : %d", discInfo.tracks);
		LogPrintf(LOG_INFO, "sides     : %d", discInfo.sides);

		if (!extended)
		{
			LogPrintf(LOG_INFO, "track size: %d", discInfo.trackSize);
		}
		else
		{
			std::set<WORD> sizes;
			std::ostringstream os;
			for (int i = 0; i < (discInfo.tracks * discInfo.sides); ++i)
			{
				const WORD trackSize = discInfo.GetTracksize(i);
				sizes.insert(trackSize);
				os << " " << std::hex << trackSize;
			}

			LogPrintf(LOG_INFO, "track sizes: %s", os.str().c_str());
			if (sizes.size() != 1)
			{
				LogPrintf(LOG_ERROR, "LoadDiskImage: Not supported: different track sizes");
				return false;
			}
		}

		Geometry geometry;
		geometry.name = DISK_ID;
		geometry.cyl = discInfo.tracks;
		geometry.head = discInfo.sides;
		geometry.sect = 0;
		geometry.sectOffset = (BYTE)-1; // Will be set (0x00 | 0xC0 | 0x40) when tracks are read

		// Standard CPC disc formats ( from https://www.cpcwiki.eu/index.php/Disc_format )
		//
		//	Data:	180k single-sided
		//			40 tracks of nine 512-byte sectors each.
		//			Sectors: 0xC1-0xC9
		//			64 directory entries
		//			Usable Capacity: 178k
		//
		//	System:	180k single-sided
		//			40 tracks of nine 512-byte sectors each.
		//			Sectors: 0x41-0x49
		//			64 directory entries
		//			Two reserved tracks containing CP/M BIOS.
		//			Usable Capacity: 169k
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
				if (!trackData.isValid())
				{
					LogPrintf(LOG_ERROR, "LoadDiskImage: track[%d,%d] has invalid data", c, h);
					return false;
				}

				floppy.data.insert(floppy.data.end(), trackData.data.begin(), trackData.data.end());

				if (geometry.sectOffset != (BYTE)-1)
				{
					// Already set, sanity check
					if (geometry.sectOffset != trackData.sectorOffset)
					{
						LogPrintf(LOG_ERROR, "LoadDiskImage: Not supported: sector offset for track[%d,%d] different than previous track %d != %d", c, h, geometry.sectOffset, trackData.sectorOffset);
						return false;
					}
				}
				else
				{
					geometry.sectOffset = trackData.sectorOffset;
				}
			}
		}

		LogPrintf(LOG_INFO, "Sector Offset: %02Xh", geometry.sectOffset);

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

	dsk::DiscInfo DeviceFloppyCPC::ReadDiscInfoBlock(FILE* f)
	{
		dsk::DiscInfo info;
		if (fread(&info, sizeof(info), 1, f) != 1)
		{
			LogPrintf(LOG_WARNING, "Error reading disc information block");
			info.Clear();
		}

		return info;
	}

	dsk::TrackInfo DeviceFloppyCPC::ReadTrackInfoBlock(FILE* f)
	{
		dsk::TrackInfo info;
		if (fread(&info, sizeof(info), 1, f) != 1)
		{
			LogPrintf(LOG_WARNING, "Error reading track info");
			info.Clear();
		}

		return info;
	}

	dsk::TrackData DeviceFloppyCPC::ReadTrackData(const dsk::TrackInfo& trackInfo, FILE* f)
	{
		dsk::TrackData trackData;

		using SectorData = std::array<BYTE, 512>;

		std::map<BYTE, SectorData> sectors;
		SectorData buf;

		for (int s = 0; s < trackInfo.sectorCount; ++s)
		{
			const dsk::SectorInfo& info = trackInfo.sectorInfo[s];
			LogPrintf(LOG_INFO, "Reading data for chs=[%d,%d,%xh]", info.track, info.side, info.sectorID);

			if (fread(&buf, 512, 1, f) != 1)
			{
				LogPrintf(LOG_WARNING, "Error reading track data");
				trackData.Clear();
				break;
			}

			sectors[info.sectorID] = buf;
		}

		for (auto& sector : sectors)
		{
			LogPrintf(LOG_DEBUG, "Sector %x", sector.first);
			trackData.data.insert(trackData.data.end(), sector.second.begin(), sector.second.end());
		}

		// Get sector offset from first sector
		trackData.sectorOffset = sectors.begin()->first & 0xC0;

		const int expected = 512 * trackInfo.sectorCount;
		if (expected != trackData.data.size())
		{
			LogPrintf(LOG_WARNING, "Track Data size expected != actual (%d != %d)", expected, trackData.data.size());
			trackData.Clear();
		}

		return trackData;
	}

}
