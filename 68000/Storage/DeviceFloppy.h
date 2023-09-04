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

	enum class StepDirection { OUTER = -1, INNER = 1 };

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

		virtual void Init() { Reset(); };
		virtual void Reset();

		virtual void Tick();

		virtual bool ClearDiskImage() { return false; };
		virtual bool LoadDiskImage(const char* path) { return false; }
		virtual bool SaveDiskImage(const char* path) { return false; }

		//const FloppyDisk& GetImageInfo(BYTE drive) { assert(drive < 4); return m_images[drive]; }

		virtual bool IsActive() const { return IsDiskLoaded() && IsMotorEnabled(); }

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
		void SetMotorSpeed(WORD rpm);
		WORD GetMotorSpeed() const { return m_motorSpeed; }

		// Pulses on and off 60 times per rotation
		bool GetMotorPulse() const { return m_motorPulse; }

		// ==========
		// Head
		// ==========

		// Sets time it takes for the head to step one track (in milliseconds)
		void SetStepDelay(WORD millis);
		WORD GetStepDelay() const { return m_stepDelay; }

		// Step one track in the current StepDirection
		// (disk doesn't need to be present)
		void Step(); 

		// Set head direction (OUTER = towards track 0)
		void SetStepDirection(StepDirection dir) { m_stepDirection = dir; }

		// True if the head is currently moving between tracks
		bool IsSeeking() const { return m_isSeeking; }

		// True if the head is currently stopped at track zero
		bool IsTrack0() const { return IsActive() && !m_isSeeking && (m_currTrack == 0) ; }

		WORD GetCurrentTrack() const { return m_currTrack; }
		WORD GetCurrentSector() const { return m_currSector; }
		WORD GetCurrentHead() const { return m_currHead; }

		WORD GetHeadCount() const { return m_headCount; }
		void SetHeadCount(WORD heads); // Select single or double sided (1 or 2)
		void SelectHead(WORD head); // Heads are numbered [0..HeadCount-1]

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		const uint32_t m_clockSpeed;

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
		static constexpr WORD MIN_TRACK = 0;
		static constexpr WORD MAX_TRACK = 100;
		static constexpr WORD MIN_STEP_MS = 1;
		static constexpr WORD MAX_STEP_MS = 100;
		static constexpr WORD DEFAULT_STEP_MS = 20;
		static constexpr WORD MIN_HEADS = 1;
		static constexpr WORD MAX_HEADS = 2;
		static constexpr WORD DEFAULT_HEAD_COUNT = 1;

		WORD m_stepDelay = DEFAULT_STEP_MS;
		uint32_t m_ticksPerTrack = UINT32_MAX;
		uint32_t m_seekCounter = UINT32_MAX;
		void ResetSeekCounter() { m_seekCounter = m_ticksPerTrack; }
		
		bool m_isSeeking = false;
		StepDirection m_stepDirection = StepDirection::OUTER;
		WORD m_maxTrack = MAX_TRACK;
		WORD m_currTrack = 0;
		WORD m_currSector = 0;
		WORD m_headCount = DEFAULT_HEAD_COUNT;
		WORD m_currHead = 0;
	};
}
