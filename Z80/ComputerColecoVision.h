#pragma once

#include <Computer/ComputerBase.h>
#include "Video/VideoColecoVision.h"
#include "IO/InputEvents.h"
#include "IO/DeviceJoystickColecoVision.h"
#include "Sound/DeviceSN76489.h"

namespace emul
{
	class CPUZ80;

	class ComputerColecoVision : public ComputerBase
	{
	public:
		ComputerColecoVision();

		virtual std::string_view GetName() const override { return "ColecoVision"; };
		virtual std::string_view GetID() const override { return "colecovision"; };

		virtual void Reset() override;
		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		CPUZ80& GetCPU() const { return *((CPUZ80*)m_cpu); }
		video::VideoColecoVision& GetVideo() { return *((video::VideoColecoVision*)m_video); }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitJoystick();
		void InitRAM();
		void InitROM();
		void InitSound();
		void InitVideo();

		emul::MemoryBlock m_ram;
		emul::MemoryBlock m_rom;

		emul::MemoryBlock m_cart;

		joy::DeviceJoystickColecoVision m_joystick;

		sn76489::DeviceSN76489 m_sound;
	};
}
