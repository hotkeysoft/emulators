#pragma once

#include "CPU/CPU8086.h"
#include "CPU/Memory.h"
#include "Storage/DeviceFloppy.h"
#include "IO/InputEvents.h"
#include "Hardware/Device8254.h"
#include "Hardware/Device8255.h"
#include "Hardware/Device8259.h"
#include "Sound/DevicePCSpeaker.h"
#include "IO/DeviceJoystick.h"
#include "Video/Video.h"

#include <set>

using emul::WORD;

namespace hdd { class DeviceHardDrive; }

namespace emul
{
	class Computer : public CPU8086
	{
	public:
		virtual ~Computer();

		virtual void Init(WORD baseRAM) = 0;

		bool LoadBinary(const char* file, ADDRESS baseAddress) { return m_memory.LoadBinary(file, baseAddress); }
		
		Memory& GetMemory() { return m_memory; }
		virtual fdc::DeviceFloppy& GetFloppy() = 0;
		virtual kbd::DeviceKeyboard& GetKeyboard() = 0;

		virtual void Reboot(bool hard = false);
		void SetTurbo(bool turbo) { m_turbo = turbo; }

	protected:
		Computer(Memory& memory, MemoryMap& mmap);

		typedef std::set<std::string> VideoModes;
		virtual void InitVideo(const std::string& defaultMode, const VideoModes& supported = VideoModes());
		virtual void InitSound();
		virtual void InitPIT(pit::Device8254* pit);
		virtual void InitPIC(pic::Device8259* pic);
		virtual void InitPPI(ppi::Device8255* ppi);
		virtual void InitJoystick(WORD baseAddress, size_t baseClock);
		virtual void InitHardDrive(hdd::DeviceHardDrive* hdd);

		struct HardDriveImageInfo
		{
			bool set = false;
			BYTE type = 0;
			std::string file;
		};
		HardDriveImageInfo GetHardDriveImageInfo(int id);

		Memory m_memory;
		MemoryMap m_map;

		MemoryBlock m_hddROM;

		pit::Device8254* m_pit = nullptr;
		pic::Device8259* m_pic = nullptr;
		ppi::Device8255* m_ppi = nullptr;
		video::Video* m_video = nullptr;
		joy::DeviceJoystick* m_joystick = nullptr;
		beeper::DevicePCSpeaker m_pcSpeaker;
		hdd::DeviceHardDrive* m_hardDrive = nullptr;

		bool m_turbo = false;
	};
}
