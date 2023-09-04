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
	struct Geometry
	{
		const char* name = nullptr;
		BYTE head = 0;
		BYTE cyl = 0 ;
		BYTE sect = 0;
		BYTE sectOffset = 0;

		bool IsSet() const { return name && GetImageSize(); }
		uint32_t GetImageSize() const { return 512 * head * cyl * sect; }
		uint32_t CHS2A(BYTE c, BYTE h, BYTE s) const { return 512 * ((c * head + h) * sect + (s - sectOffset) - 1); }
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

	class DeviceFloppy : public emul::Serializable, public Logger
	{
	protected:
		DeviceFloppy(uint32_t clockSpeedHz);

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

		virtual bool ClearDiskImage() { return false; };
		virtual bool LoadDiskImage(const char* path) { return false; }
		virtual bool SaveDiskImage(const char* path) { return false; }

		//const FloppyDisk& GetImageInfo(BYTE drive) { assert(drive < 4); return m_images[drive]; }

		virtual bool IsActive() const { return IsDiskLoaded() && IsMotorEnabled(); }

		// Disk
		bool IsDiskLoaded() const { return m_diskLoaded; }
		bool IsDiskChanged() const { return m_diskChanged; }
		void ClearDiskChanged() { m_diskChanged = false; }

		// Rotation
		bool IsMotorEnabled() const { return m_motorEnabled; }
		void EnableMotor(bool enable);
		void SetMotorRPM(WORD speed);
		WORD GetMotorRPM() const { return m_motorSpeed; }
		bool GetMotorPulse() const { return m_motorPulse; }

		// Head
		bool IsSeeking() const { return m_isSeeking; }
		bool IsTrack0() const { return IsActive() && !m_isSeeking && (m_track == 0) ; }
		WORD GetCurrentTrack() const { return m_track; }
		WORD GetCurrentSector() const { return m_sector; }
		WORD GetActiveHead() const { return m_head; }
		void SelectHead(WORD head) { m_head = head; }
		void Seek(WORD track);

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		const uint32_t m_clockSpeed;
		uint32_t m_currOpWait = 0;

		uint32_t DelayToTicks(uint32_t delayMS);

		// Disk
		bool m_diskLoaded = false;
		bool m_diskChanged = false;

		// Rotation
		static constexpr WORD MIN_RPM = 60;
		static constexpr WORD MAX_RPM = 1000;
		static constexpr WORD DEFAULT_RPM = 360;
		WORD m_motorSpeed = DEFAULT_RPM;
		bool m_motorEnabled = false;

		void ResetPulseCounter() { m_motorPulseCounter = m_ticksPerPulse; }
		uint32_t m_ticksPerPulse = UINT32_MAX;
		uint32_t m_motorPulseCounter = UINT32_MAX;
		bool m_motorPulse = false; // Ticks on/off 60 times per revolution

		// Head
		bool m_isSeeking = false;
		WORD m_track = 0;
		WORD m_sector = 0;
		WORD m_head = 0;
	};
}
