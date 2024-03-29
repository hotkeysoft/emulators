#pragma once

#include "CPU/CPU8086.h"
#include <Computer/ComputerBase.h>
#include <CPU/Memory.h>
#include "IO/InputEvents.h"
#include "Hardware/DeviceDMAPageRegister.h"
#include "Sound/DevicePCSpeaker.h"
#include "Video/Video.h"
#include <Serializable.h>

#include <set>

using emul::WORD;

namespace dma { class Device8237; }
namespace events { class InputEvents; }
namespace fdc { class DeviceFloppy; }
namespace hdd { class DeviceHardDrive; }
namespace joy { class DeviceJoystick; }
namespace mouse { class DeviceSerialMouse; }
namespace pic { class Device8259; }
namespace pit { class Device8254; }
namespace ppi { class DevicePPI; }
namespace rtc { class Device8167; }

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

	class Computer : public ComputerBase
	{
	public:
		const char* CPUID_8086 = "8086";
		const char* CPUID_80186 = "80186";
		const char* CPUID_80286 = "80286";

		virtual ~Computer();

		virtual std::string_view GetModel() const override
		{
			static std::string model;
			// Use video mode as model only if non-trivial (e.g skip pcjr)
			const auto& videoModes = GetVideoModes();
			if (videoModes.size() > 1)
			{
				model = GetVideo().GetDisplayName();
			}
			return model;
		}

		virtual void Init(WORD baseRAM) = 0;

		bool LoadBinary(const char* file, ADDRESS baseAddress) { return m_memory.LoadBinary(file, baseAddress); }

		beeper::DevicePCSpeaker& GetSound() { return m_pcSpeaker; } // TODO: Sound interface
		fdc::DeviceFloppy* GetFloppy() { return m_floppy; }
		hdd::DeviceHardDrive* GetHardDrive() { return m_hardDrive; }
		virtual kbd::DeviceKeyboard& GetKeyboard() = 0;
		mouse::DeviceSerialMouse* GetMouse() { return m_mouse; }

		virtual void Reboot(bool hard = false);

		CPU8086* GetCPU() const { return (CPU8086*)m_cpu; }

		typedef std::set<CPUSpeed> CPUSpeeds;
		CPUSpeeds GetCPUSpeeds() const { return m_cpuSpeeds; }
		CPUSpeed GetCPUSpeed() const { return m_cpuSpeed; }
		void SetCPUSpeed(const CPUSpeed& speed);

		typedef std::set<std::string> VideoModes;
		const VideoModes& GetVideoModes() const { return m_videoModes; }

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		Computer();

		virtual void Init(const char* cpuid, WORD baseRAM) override;
		virtual void InitCPU(const char* cpuid) override;

		virtual void InitVideo(const std::string& defaultMode, const VideoModes& supported = VideoModes());
		virtual void InitSound();
		virtual void InitPIT(pit::Device8254* pit);
		virtual void InitPIC(pic::Device8259* pic);
		virtual void InitPPI(ppi::DevicePPI* ppi);
		virtual void InitDMA(dma::Device8237* dmaPrimary, dma::Device8237* dmaSecondary = nullptr);
		virtual void InitJoystick(WORD baseAddress, size_t baseClock);
		virtual void InitFloppy(fdc::DeviceFloppy* fdd, BYTE irq=0, BYTE dma=0);
		virtual void InitHardDrive(hdd::DeviceHardDrive* hdd, BYTE irq = 0, BYTE dma = 0);
		virtual void InitMouse(size_t baseClock);
		virtual void InitRTC();

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

		MemoryBlock m_hddROM;

		pit::Device8254* m_pit = nullptr;
		pic::Device8259* m_pic = nullptr;
		ppi::DevicePPI* m_ppi = nullptr;
		dma::Device8237* m_dma1 = nullptr;
		dma::Device8237* m_dma2 = nullptr;
		dma::DeviceDMAPageRegister m_dmaPageRegister;
		joy::DeviceJoystick* m_joystick = nullptr;
		beeper::DevicePCSpeaker m_pcSpeaker;
		hdd::DeviceHardDrive* m_hardDrive = nullptr;
		fdc::DeviceFloppy* m_floppy = nullptr;
		mouse::DeviceSerialMouse* m_mouse = nullptr;
		rtc::Device8167* m_rtc = nullptr;

		BYTE m_floppyIRQ = 0;
		BYTE m_floppyDMA = 0;
		BYTE m_hddIRQ = 0;
		BYTE m_hddDMA = 0;

	private:
		VideoModes m_videoModes;

		CPUSpeed m_cpuSpeed;
		CPUSpeeds m_cpuSpeeds;
	};
}
