#pragma once

#include "CPU/CPU8086.h"
#include "CPU/Memory.h"
#include "Storage/DeviceFloppy.h"
#include "IO/InputEvents.h"
#include "Hardware/Device8254.h"
#include "Hardware/Device8255.h"
#include "Hardware/Device8259.h"
#include "Hardware/Device8237.h"
#include "Sound/DevicePCSpeaker.h"
#include "IO/DeviceJoystick.h"
#include "IO/DeviceSerialMouse.h"
#include "Video/Video.h"
#include "Serializable.h"

#include <set>

using emul::WORD;

namespace hdd { class DeviceHardDrive; }

namespace emul
{
	static const BYTE COM_IRQ[5] = { 0, 4, 3, 4, 3 };
	static const WORD COM_PORT[5] = { 0, 0x3F8, 0x2F8, 0x3E8, 0x2E8 };

	class CPUSpeed : public Serializable
	{
	public:
		CPUSpeed() = default;
		CPUSpeed(size_t baseSpeed, int ratio) : m_ratio(ratio), m_speed(baseSpeed * ratio) {}
		size_t GetSpeed() const { return m_speed; }
		int GetRatio() const { return m_ratio; }

		bool operator<(const CPUSpeed& other) const { return this->m_ratio < other.m_ratio; }

		// emul::Serializable
		virtual void Serialize(json& to)
		{
			to["ratio"] = m_ratio;
			to["speed"] = m_speed;
		}
		virtual void Deserialize(const json& from)
		{
			m_ratio = from["ratio"];
			m_speed = from["speed"];
		}

	protected:
		int m_ratio = 4;
		size_t m_speed = 4772726;
	};

	class Computer : public CPU8086
	{
	public:
		virtual ~Computer();

		virtual std::string_view GetName() const = 0;
		virtual std::string_view GetID() const = 0;

		virtual void Init(WORD baseRAM);

		bool LoadBinary(const char* file, ADDRESS baseAddress) { return m_memory.LoadBinary(file, baseAddress); }
		
		Memory& GetMemory() { return m_memory; }
		beeper::DevicePCSpeaker& GetSound() { return m_pcSpeaker; } // TODO: Sound interface
		fdc::DeviceFloppy* GetFloppy() { return m_floppy; }
		hdd::DeviceHardDrive* GetHardDrive() { return m_hardDrive; }
		virtual kbd::DeviceKeyboard& GetKeyboard() = 0;
		video::Video& GetVideo() { return *m_video; }
		events::InputEvents& GetInputs() { return *m_inputs; }
		mouse::DeviceSerialMouse* GetMouse() { return m_mouse; }
		

		virtual void Reboot(bool hard = false);
		void SetTurbo(bool turbo) { m_turbo = turbo; }

		typedef std::set<CPUSpeed> CPUSpeeds;
		CPUSpeeds GetCPUSpeeds() const { return m_cpuSpeeds; }
		CPUSpeed GetCPUSpeed() const { return m_cpuSpeed; }
		void SetCPUSpeed(const CPUSpeed& speed);

		typedef std::set<std::string> VideoModes;
		const VideoModes& GetVideoModes() const { return m_videoModes; }

		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		Computer(Memory& memory, MemoryMap& mmap);

		virtual void InitVideo(const std::string& defaultMode, const VideoModes& supported = VideoModes());
		virtual void InitSound();
		virtual void InitPIT(pit::Device8254* pit);
		virtual void InitPIC(pic::Device8259* pic);
		virtual void InitPPI(ppi::Device8255* ppi);
		virtual void InitDMA(dma::Device8237* dma);
		virtual void InitJoystick(WORD baseAddress, size_t baseClock);
		virtual void InitFloppy(fdc::DeviceFloppy* fdd, BYTE irq=0, BYTE dma=0);
		virtual void InitHardDrive(hdd::DeviceHardDrive* hdd, BYTE irq = 0, BYTE dma = 0);
		virtual void InitInputs(size_t clockSpeedHz);
		virtual void InitMouse(size_t baseClock);

		void AddCPUSpeed(const CPUSpeed& speed);
		int GetCPUSpeedRatio() const { return m_cpuSpeed.GetRatio(); }

		struct HardDriveImageInfo
		{
			bool set = false;
			BYTE type = 0;
			std::string file;
		};
		HardDriveImageInfo GetHardDriveImageInfo(int id);

		virtual void TickFloppy();
		virtual void TickHardDrive();

		WORD m_baseRAM = 640;
		Memory m_memory;
		MemoryMap m_map;

		MemoryBlock m_hddROM;

		pit::Device8254* m_pit = nullptr;
		pic::Device8259* m_pic = nullptr;
		ppi::Device8255* m_ppi = nullptr;
		dma::Device8237* m_dma = nullptr;
		video::Video* m_video = nullptr;
		joy::DeviceJoystick* m_joystick = nullptr;
		beeper::DevicePCSpeaker m_pcSpeaker;
		hdd::DeviceHardDrive* m_hardDrive = nullptr;
		fdc::DeviceFloppy* m_floppy = nullptr;
		events::InputEvents* m_inputs = nullptr;
		mouse::DeviceSerialMouse* m_mouse = nullptr;

		BYTE m_floppyIRQ = 0;
		BYTE m_floppyDMA = 0;
		BYTE m_hddIRQ = 0;
		BYTE m_hddDMA = 0;

		bool m_turbo = false;

	private:
		VideoModes m_videoModes;

		CPUSpeed m_cpuSpeed;
		CPUSpeeds m_cpuSpeeds;
	};
}
