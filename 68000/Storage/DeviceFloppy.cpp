#include "stdafx.h"

#include "DeviceFloppy.h"
#include <array>
#include <FileUtil.h>

using hscommon::fileUtil::File;
using emul::GetBit;
using emul::SetBit;
using emul::GetMSB;
using emul::SetLSB;

namespace fdd
{
	static constexpr std::array<BYTE, 6> SYNC_FIELD = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	static constexpr std::array<BYTE, 3> HEADER_BEGIN = { 0xD5, 0xAA, 0x96 };
	static constexpr std::array<BYTE, 3> HEADER_END = { 0xDE, 0xAA, 0x00 };
	static constexpr std::array<BYTE, 3> DATA_BEGIN = { 0xD5, 0xAA, 0xAD };
	static constexpr std::array<BYTE, 3> DATA_END = { 0xDE, 0xAA, 0x00 };

	static constexpr BYTE EncodingTable6and2[] = {
		0x96, 0x97, 0x9A, 0x9B, 0x9D, 0x9E, 0x9F, 0xA6,
		0xA7, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB2, 0xB3,
		0xB4, 0xB5, 0xB6, 0xB7, 0xB9, 0xBA, 0xBB, 0xBC,
		0xBD, 0xBE, 0xBF, 0xCB, 0xCD, 0xCE, 0xCF, 0xD3,
		0xD6, 0xD7, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE,
		0xDF, 0xE5, 0xE6, 0xE7, 0xE9, 0xEA, 0xEB, 0xEC,
		0xED, 0xEE, 0xEF, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6,
		0xF7, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
	};

	BYTE Encode6and2(BYTE data) { assert(data < 64); return EncodingTable6and2[data]; }

	BYTE GetHigh2(const BYTE b) { return b & 0b11000000; }
	BYTE GetLow6(const BYTE b) { return b & 0b00111111; }

	const char* GetStepDirectionStr(StepDirection dir)
	{
		switch (dir)
		{
		case StepDirection::OUTER: return "OUTER";
		case StepDirection::INNER: return "INNER";
		default:
			NODEFAULT;
		}
	}

	const FloppyDisk::Interleave FloppyDisk::s_speedGroups[5] = {
		{ 0, 6, 1, 7, 2, 8, 3, 9, 4, 10, 5, 11 }, // Tracks[0..15]
		{ 0, 6, 1, 7, 2, 8, 3, 9, 4, 10, 5 },     // Tracks[16..31]
		{ 0, 5, 1, 6, 2, 7, 3, 8, 4, 9 },         // Tracks[32..47]
		{ 0, 5, 1, 6, 2, 7, 3, 8, 4 },            // Tracks[48..63]
		{ 0, 4, 1, 5, 2, 6, 3, 7 }                // Tracks[64..79]
	};

	void FloppyDisk::Init(DiskFormat format)
	{
		m_tracks.clear();
		m_tracks.resize(80);
	}

	const FloppyDisk::Interleave& FloppyDisk::GetInterleave(int track) const
	{
		if (track < 0 || track >= 80)
		{
			LogPrintf(LOG_ERROR, "GetInterleave(): Invalid Track number");
			throw std::exception("Invalid track number");
		}
		return s_speedGroups[track / 16];
	}

	int FloppyDisk::GetSectorCount(int track) const
	{
		if (track < 0 || track >= 80)
		{
			LogPrintf(LOG_ERROR, "GetInterleave(): Invalid Track number");
			throw std::exception("Invalid track number");
		}
		return (int)s_speedGroups[track / 16].size();
	}

	bool FloppyDisk::Load(std::filesystem::path imageFilePath)
	{
		size_t size = std::filesystem::file_size(imageFilePath);

		// TODO
		constexpr size_t IMAGE_SIZE_400k = 800 * 512;
		if (size != IMAGE_SIZE_400k)
		{
			LogPrintf(LOG_ERROR, "Image size mismatch (expected %zu, got %zu)", IMAGE_SIZE_400k, size);
			return false;
		}

		Init(DiskFormat::Mac400K);
		hscommon::fileUtil::File imageFile(imageFilePath.string().c_str(), "rb");

		WORD block = 0;

		for (int i = 0; i < m_tracks.size(); ++i)
		{
			const int sectorsPerTrack = GetSectorCount(i);
			LogPrintf(LOG_INFO, "Track [%d]: Loading [%d] sectors", i, sectorsPerTrack);
			RawSectors sectors;
			for (int s = 0; s < sectorsPerTrack; ++s)
			{
				RawSectorData sector;
				sector.tag.blockNumber = block++;
				size_t ret = fread(sector.data, SECTOR_DATA_SIZE, 1, imageFile);
				if (ret != 1)
				{
					LogPrintf(LOG_ERROR, "Error reading image data");
					return false;
				}
				sectors.push_back(sector);
			}
			m_tracks[i] = sectors;
		}

		return true;
	}

	const RawSectors& FloppyDisk::GetTrack(int track)
	{
		if (track < 0 || track >= m_tracks.size())
		{
			throw std::exception("Invalid track number");
		}

		return m_tracks[track];
	}

	DeviceFloppy::DeviceFloppy(uint32_t clockSpeedHz, bool connected) :
		Logger("fdd"),
		m_clockSpeed(clockSpeedHz),
		m_connected(connected)
	{
	}

	void DeviceFloppy::EnableLog(Logger::SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		m_disk.EnableLog(minSev);
	}

	void DeviceFloppy::Reset()
	{
		LogPrintf(LOG_INFO, "Reset");
		LogPrintf(LOG_INFO, "Clock Speed = %d, connected = %d", m_clockSpeed, m_connected);
		
		Logger::SEVERITY oldSev = GetLogLevel();
		EnableLog(LOG_OFF);

		m_diskLoaded = false;
		m_diskChanged = false;
		m_motorEnabled = false;
		m_motorPulse = false;
		m_seekCounter = UINT32_MAX;
		m_isSeeking = false;
		m_isCalibrating = false;
		m_stepDirection = StepDirection::INNER;
		m_currTrack = 0;
		m_currHead = 0;
		m_currSectorIndex = 0;

		// Set only if never uninitialized
		if (m_ticksPerRotation == UINT32_MAX)
		{
			SetMotorSpeed(DEFAULT_RPM, true);
		}

		if (m_ticksPerTrack == UINT32_MAX)
		{
			SetStepDelay(DEFAULT_STEP_MS);
		}

		ClearDiskChanged();

		EnableLog(oldSev);
	}

	void DeviceFloppy::Tick()
	{
		if (!m_connected)
			return;

		if (m_motorEnabled && (--m_motorPulseCounter == 0))
		{
			ResetPulseCounter();
			m_motorPulse = !m_motorPulse;
		}

		if (m_isSeeking && (--m_seekCounter == 0))
		{
			ResetSeekCounter();
			m_isSeeking = false;
			int newTrack = m_currTrack + (int)m_stepDirection;
			// Allow one more inner track to signify "out of bounds"
			int adjustedTrack =  std::clamp(newTrack, 0, (int)m_trackCount); 
			if (newTrack != adjustedTrack)
			{
				LogPrintf(LOG_WARNING, "Seek past endpoint");
			}

			LogPrintf(LOG_INFO, "Seek End, new track: [%d]", adjustedTrack);
			NewTrack(adjustedTrack);
		}
	}

	bool DeviceFloppy::LoadDiskImage(const char* path)
	{
		if (!m_disk.Load(path))
		{
			return false;
		}

		// Prepare the track/sector data
		for (int track = 0; track < m_disk.GetTrackCount(); ++track)
		{
			DATA currTrack;
			const size_t sectorCount = m_disk.GetSectorCount(track);
			const auto& interleave = m_disk.GetInterleave(track);
			const auto& rawTrackData = m_disk.GetTrack(track);

			EncodedTrack encodedTrack;
			encodedTrack.reserve(12);

			for (int sector : interleave)
			{
				LogPrintf(LOG_INFO, "Prepare data for track %d sector %d", track, sector);

				EncodedSector encodedSector;
				encodedSector.sector = sector;

				auto& data = encodedSector.data;
				data.reserve(800);

				// Padding
				data.insert(data.end(), SYNC_FIELD.begin(), SYNC_FIELD.end());

				// Sector Header
				DATA header = BuildHeader(track, sector, 0, DiskFormat::Mac400K);
				data.insert(data.end(), header.begin(), header.end());

				// Padding
				data.insert(data.end(), SYNC_FIELD.begin(), SYNC_FIELD.end());

				// Sector Data
				DATA sectorData = BuildData(sector, rawTrackData[sector]);
				data.insert(data.end(), sectorData.begin(), sectorData.end());

				// Padding
				data.insert(data.end(), SYNC_FIELD.begin(), SYNC_FIELD.end());

				encodedTrack.push_back(encodedSector);
			}

			m_trackData.push_back(encodedTrack);
		}

		NewTrack(0);

		return true;
	}

	void DeviceFloppy::EnableMotor(bool enable)
	{
		if (!m_connected)
			return;

		LogPrintf(LOG_INFO, "Motor: %s", enable ? "ON" : "OFF");

		if (enable && !m_motorEnabled)
		{
			ResetPulseCounter();
		}
		m_motorEnabled = enable;
	}

	void DeviceFloppy::SetMotorSpeed(WORD rpm, bool force) 
	{ 
		if (!force && (!m_connected || (rpm == m_motorSpeed)))
			return;

		LogPrintf(LOG_DEBUG, "Set Motor Speed: %d rpm", rpm);
		m_motorSpeed = std::clamp(rpm, MIN_RPM, MAX_RPM);
		if (m_motorSpeed != rpm)
		{
			LogPrintf(LOG_WARNING, "Motor Speed clamped to %d rpm", m_motorSpeed);
		}
		m_ticksPerRotation = m_clockSpeed / m_motorSpeed / 2;
		LogPrintf(LOG_DEBUG, "Ticks per Rotation: %d", m_ticksPerRotation);

		if (m_motorPulseCounter > m_ticksPerRotation)
		{
			ResetPulseCounter();
		}
	}

	void DeviceFloppy::SetHeadCount(WORD heads)
	{
		if (!m_connected)
			return;

		LogPrintf(LOG_INFO, "Set Head Count: %d", heads);
		m_headCount = std::clamp(heads, MIN_HEADS, MAX_HEADS);
		if (m_headCount != heads)
		{
			LogPrintf(LOG_WARNING, "Head Count clamped to %d", m_headCount);
		}
		m_currHead = 0;
	}
	// Heads are numbered [0..HeadCount-1]
	void DeviceFloppy::SelectHead(WORD head)
	{
		if (!m_connected)
			return;

		LogPrintf(LOG_INFO, "Select Head: %d", head);
		if (head >= m_headCount)
		{
			LogPrintf(LOG_ERROR, "Invalid Head: %d (max = %d)", head, m_headCount - 1);
			m_currHead = 0;
		}
		else
		{
			m_currHead = head;
		}
	}

	void DeviceFloppy::SetTrackCount(WORD tracks)
	{
		if (!m_connected)
			return;

		LogPrintf(LOG_INFO, "Set Track Count: %d", tracks);
		m_trackCount = std::clamp(tracks, MIN_TRACKS, MAX_TRACKS);
		if (m_trackCount != tracks)
		{
			LogPrintf(LOG_WARNING, "Track Count clamped to %d", m_headCount);
		}
		m_currTrack = 0;
	}

	void DeviceFloppy::SetStepDelay(WORD millis)
	{
		if (!m_connected)
			return;

		LogPrintf(LOG_INFO, "Set Step Delay: %d ms", millis);
		m_stepDelay = std::clamp(millis, MIN_STEP_MS, MAX_STEP_MS);
		if (m_stepDelay != millis)
		{
			LogPrintf(LOG_WARNING, "Step Delay clamped to %d ms", m_stepDelay);
		}

		m_ticksPerTrack = DelayToTicks(m_stepDelay);
		LogPrintf(LOG_DEBUG, "Ticks per Track: %d", m_ticksPerTrack);

		if (m_seekCounter > m_ticksPerTrack)
		{
			ResetSeekCounter();
		}
	}

	void DeviceFloppy::SetStepDirection(StepDirection dir)
	{ 
		if (!m_connected)
			return;

		LogPrintf(LOG_INFO, "Set Step Direction: %s", GetStepDirectionStr(dir));
		m_stepDirection = dir; 
	}

	void DeviceFloppy::Step()
	{
		if (!m_connected)
			return;

		LogPrintf(LOG_INFO, "Step one track in [%s] direction", GetStepDirectionStr(m_stepDirection));

		if (m_isSeeking)
		{
			LogPrintf(LOG_WARNING, "Step: Already seeking, ignored");
			return;
		}

		m_isSeeking = true;
		ResetSeekCounter();
	}

	void DeviceFloppy::NewTrack(WORD track)
	{
		LogPrintf(LOG_INFO, "NewTrack [%d]", track);
		m_currTrack = track;

		m_currSectorPos = 0;
		m_currSectorIndex = 0;
		m_currSectorData = &m_trackData[track][0];
		LogPrintf(LOG_INFO, "Begin Sector [%d]", m_currSectorData->sector);
	}

	void DeviceFloppy::NextSector()
	{
		auto& currTrack = m_trackData[m_currTrack];

		++m_currSectorIndex;
		if (m_currSectorIndex == currTrack.size())
		{
			m_currSectorIndex = 0;
		}

		LogPrintf(LOG_INFO, "Begin Sector [%d]", currTrack[m_currSectorIndex].sector);
		m_currSectorPos = 0;
		m_currSectorData = &currTrack[m_currSectorIndex];
	}

	BYTE DeviceFloppy::ReadByte()
	{
		// TODO: Feed bytes at proper speed

		if (m_currSectorPos == m_currSectorData->data.size())
		{
			NextSector();
		}

		BYTE value = m_currSectorData->data.at(m_currSectorPos++);

		LogPrintf(LOG_DEBUG, "Read Data, value = %02X", value);

		return value;
	}

	DATA DeviceFloppy::BuildHeader(BYTE track, BYTE sector, BYTE side, DiskFormat format)
	{
		BYTE trackLow = GetLow6(track); // Bits 0..5
		bool trackHi = GetBit(track, 6); // Bit 6
		if (GetBit(track, 7))
		{
			LogPrintf(LOG_ERROR, "Track > 127, cannot be encoded in header");
			throw std::exception("Invalid track number");
		}

		if (side > 1)
		{
			LogPrintf(LOG_ERROR, "Side > 1 cannot be encoded in header");
			throw std::exception("Invalid side number");
		}

		if (sector > 63)
		{
			LogPrintf(LOG_ERROR, "Sector > 63 cannot be encoded in header");
			throw std::exception("Invalid sector number");
		}

		DATA data;
		BYTE checksum = 0;

		// Header Begin
		data.insert(data.end(), HEADER_BEGIN.begin(), HEADER_BEGIN.end());

		// Track number (low 6 bits)
		{
			checksum ^= trackLow;
			BYTE encoded = Encode6and2(trackLow);
			data.push_back(encoded);

		}

		// Sector number (6 bits)
		{
			checksum ^= sector;
			BYTE encoded = Encode6and2(sector);
			data.push_back(encoded);
		}

		// Side (and upper track bit)
		{
			BYTE sideData = 0;
			SetBit(sideData, 0, trackHi);
			SetBit(sideData, 5, side);
			checksum ^= sideData;

			BYTE encoded = Encode6and2(sideData);
			data.push_back(encoded);
		}

		// Format
		{
			checksum ^= (BYTE)format;
			BYTE encoded = Encode6and2((BYTE)format);

			data.push_back(encoded);
		}

		// Checksum
		{
			BYTE encoded = Encode6and2(checksum);
			data.push_back(encoded);
		}

		// Header End
		data.insert(data.end(), HEADER_END.begin(), HEADER_END.end());
		return data;
	}

	DATA DeviceFloppy::BuildData(BYTE sector, const RawSectorData& rawData)
	{
		if (sector > 63)
		{
			LogPrintf(LOG_ERROR, "Sector > 63 cannot be encoded in header");
			throw std::exception("Invalid sector number");
		}

		DATA data;
		data.reserve(710);

		// Header Begin
		data.insert(data.end(), DATA_BEGIN.begin(), DATA_BEGIN.end());

		// Sector number (6 bits)
		{
			BYTE encoded = Encode6and2(sector);
			data.push_back(encoded);
		}

		ResetChecksum();
		// Sector Tag (12 bytes)
		{
			DATA encodedTag = EncodeDataBlock((BYTE*)&rawData.tag, SECTOR_TAG_SIZE);
			if (encodedTag.size() != 16)
			{
				LogPrintf(LOG_ERROR, "Data Tag should encode to 16 bytes");
				throw std::exception("Invalid encoded tag size");
			}
			data.insert(data.end(), encodedTag.begin(), encodedTag.end());
		}

		// Sector Data (512 bytes)
		{
			DATA encodedData = EncodeDataBlock(rawData.data, SECTOR_DATA_SIZE);
			if (encodedData.size() != 683)
			{
				LogPrintf(LOG_ERROR, "Data Tag should encode to 683 bytes");
				throw std::exception("Invalid encoded block size");
			}
			data.insert(data.end(), encodedData.begin(), encodedData.end());
		}

		// Checksum
		{
			BYTE hiA = GetHigh2(m_checksumA) >> 2;
			BYTE hiB = GetHigh2(m_checksumB) >> 4;
			BYTE hiC = GetHigh2(m_checksumC) >> 6;

			data.push_back(Encode6and2(hiA | hiB | hiC));
			data.push_back(Encode6and2(GetLow6(m_checksumA)));
			data.push_back(Encode6and2(GetLow6(m_checksumB)));
			data.push_back(Encode6and2(GetLow6(m_checksumC)));
		}

		// Header End
		data.insert(data.end(), DATA_END.begin(), DATA_END.end());
		return data;
	}

	void DeviceFloppy::Encode3To4(BYTE encodedOut[4], BYTE byteA, BYTE byteB, BYTE byteC, bool lastBlock)
	{
		// 1. Rotate CSUMC left
		bool carry1 = ROL(m_checksumC);

		// 2. CSUMA <- CSUMA + BYTEA + carry from step 1
		bool carry2 = ADC(m_checksumA, byteA, carry1);

		// 3. BYTEA <- BYTEA xor CSUMC
		byteA ^= m_checksumC;

		// 4. CSUMB <- CSUMB + BYTEB + carry from step 2
		bool carry4 = ADC(m_checksumB, byteB, carry2);

		// 5. BYTEB <- BYTEB xor CSUMA
		byteB ^= m_checksumA;

		if (!lastBlock)
		{
			// 6. CSUMC <- CSUMC + BYTEC + carry from step 4
			bool carry6 = ADC(m_checksumC, byteC, carry4);

			// 7. BYTEB <- BYTEC xor CSUMB
			byteC ^= m_checksumB;
		}

		// 8. convert BYTEA, BYTEB and BYTEC to 6 bit nibbles
		//   NIBL1 <- A7 A6 B7 B6 C7 C6 : High bits of the bytes
		//   NIBL2 <- A5 A4 A3 A2 A1 A0 : Low bits of BYTEA
		//   NIBL3 <- B5 B4 B3 B2 B1 B0 : Low bits of BYTEB
		//   NIBL4 <- C5 C4 C3 C2 C1 C0 : Low bits of BYTEC

		BYTE hiA = GetHigh2(byteA) >> 2;
		BYTE hiB = GetHigh2(byteB) >> 4;
		BYTE hiC = GetHigh2(byteC) >> 6;

		encodedOut[0] = hiA | hiB | hiC;
		encodedOut[1] = GetLow6(byteA);
		encodedOut[2] = GetLow6(byteB);
		encodedOut[3] = GetLow6(byteC);
	}

	DATA DeviceFloppy::EncodeDataBlock(const BYTE data[], int size)
	{
		DATA encoded;

		// 3 raw bytes in -> 4 encoded bytes out
		// Work with chunks of 3 bytes
		int pos = 0;

		BYTE encodedBlock[4];
		while (pos < (size / 3) * 3)
		{
			BYTE byteA = data[pos++];
			BYTE byteB = data[pos++];
			BYTE byteC = data[pos++];

			Encode3To4(encodedBlock, byteA, byteB, byteC);

			encoded.push_back(Encode6and2(encodedBlock[0]));
			encoded.push_back(Encode6and2(encodedBlock[1]));
			encoded.push_back(Encode6and2(encodedBlock[2]));
			encoded.push_back(Encode6and2(encodedBlock[3]));
		}

		// Possible last block of 2 bytes (sector = 512 bytes == 170 * 3 + 2)
		int left = size - pos;
		if (left == 1)
		{
			// Should not happen with the sizes we're dealing with
			throw std::exception("Invalid block size");
		}
		else if (left == 2)
		{
			BYTE byteA = data[pos++];
			BYTE byteB = data[pos++];

			Encode3To4(encodedBlock, byteA, byteB, 0, true);

			encoded.push_back(Encode6and2(encodedBlock[0]));
			encoded.push_back(Encode6and2(encodedBlock[1]));
			encoded.push_back(Encode6and2(encodedBlock[2]));
		}

		return encoded;
	}

	bool DeviceFloppy::ADC(BYTE& dest, BYTE src, bool carryIn)
	{
		int res = dest + src + carryIn;
		dest = (BYTE)res;
		return res > 0xFF;
	}
	bool DeviceFloppy::ROL(BYTE& dest)
	{
		bool rot = GetMSB(dest);
		dest <<= 1;
		SetBit(dest, 0, rot);

		return rot;
	}

	void DeviceFloppy::Serialize(json& to)
	{

	}
	void DeviceFloppy::Deserialize(const json& from)
	{

	}
}