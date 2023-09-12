#pragma once

#include <Computer/ComputerBase.h>
#include "IO/InputEvents.h"
#include "Hardware/Device6522Mac.h"
#include "Hardware/IOBlockVIAMac.h"
#include "Hardware/DeviceIWM.h"
#include "Hardware/IOBlockIWMMac.h"
#include "Hardware/Device8530.h"
#include "Hardware/IOBlockSCCMac.h"
#include "IO/DeviceMouseMac.h"
#include "Video/VideoMac.h"
#include "Sound/SoundMac.h"
#include "Storage/DeviceFloppy.h"

namespace emul
{
	namespace cpu68k { class CPU68000; };

	class ComputerMacintosh :
		public ComputerBase,
		public video::mac::EventHandler,
		public via::mac::EventHandler
	{
	public:
		ComputerMacintosh();

		virtual std::string_view GetName() const override { return "Macintosh"; };
		virtual std::string_view GetID() const override { return "mac"; };

		virtual void Init(WORD baseRAM) override;
		virtual void Reset() override;

		virtual bool Step() override;

		cpu68k::CPU68000& GetCPU() const { return *((cpu68k::CPU68000*)m_cpu); }

		video::mac::VideoMac& GetVideo() { return *((video::mac::VideoMac*)m_video); }

	protected:
		virtual void InitCPU(const char* cpuid) override;
		void InitVIA();
		void InitSCC();
		void InitFloppy();
		void InitMouse();

		void InitVideo();

		void SetROMOverlayMode(bool overlay);

		void SetFloppySpeed(int pwm);

		// video::mac::EventHandler
		virtual void OnHBlankStart() override;
		virtual void OnVBlankStart() override;

		// via::mac::EventHandler
		virtual void OnSoundResetChange(bool reset) override;
		virtual void OnSoundBufferChange(bool mainBuffer) override;
		virtual void OnVideoPageChange(bool mainBuffer) override;
		virtual void OnHeadSelChange(bool selectedHead) override;
		virtual void OnROMOverlayModeChange(bool overlay) override;

		constexpr static ADDRESS RAM_SIZE = 0x20000;
		constexpr static ADDRESS ROM_SIZE = 0x10000;

		constexpr static ADDRESS ROM_BASE = 0x400000;
		constexpr static ADDRESS ROM_BASE_OVERLAY = 0x000000;

		constexpr static ADDRESS ROM_MIRROR_BASE = 0x600000;
		constexpr static ADDRESS ROM_MIRROR_BASE_OVERLAY = 0x200000;

		constexpr static ADDRESS RAM_BASE = 0x000000;
		constexpr static ADDRESS RAM_BASE_OVERLAY = 0x600000;

		constexpr static ADDRESS RAM_VIDEO_PAGE1_OFFSET = 0x01A700;
		constexpr static ADDRESS RAM_VIDEO_PAGE2_OFFSET = 0x012700;
		constexpr static ADDRESS RAM_SOUND_PAGE1_OFFSET = 0x01FD00;
		constexpr static ADDRESS RAM_SOUND_PAGE2_OFFSET = 0x01A100;

		ADDRESS m_ramBaseAddress = RAM_BASE;

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_rom;

		via::mac::IOBlockVIAMac m_ioVIA;
		via::mac::Device6522Mac m_via;

		floppy::woz::IOBlockIWMMac m_ioIWM;
		floppy::woz::DeviceIWM m_floppyController;
		fdd::DeviceFloppy m_floppyInternal;
		fdd::DeviceFloppy m_floppyExternal;

		scc::mac::IOBlockSCCMac m_ioSCC;
		scc::Device8530 m_scc;

		mouse::mac::DeviceMouseMac m_mouse;

		sound::mac::SoundMac m_sound;
	};
}
