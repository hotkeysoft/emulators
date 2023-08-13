#pragma once

#include <Computer/ComputerBase.h>
#include <CPU/IOBlock.h>
#include "IO/InputEvents.h"
#include "Hardware/Device6520MO5_PIA.h"
#include "IO/DeviceKeyboardThomson.h"
#include "IO/DeviceLightpenThomson.h"
#include "Video/VideoThomson.h"

namespace emul
{
	class CPU6809;

	class ComputerThomson : public ComputerBase, public IOConnector, public pia::EventHandler
	{
	public:
		ComputerThomson();

		virtual std::string_view GetName() const override { return "Thomson"; };
		virtual std::string_view GetID() const override { return "thomson"; };

		virtual std::string_view GetModel() const override { return "MO5"; };

		virtual void Init(WORD baseRAM) override;
		virtual void Reset() override;
		virtual bool Step() override;

		CPU6809& GetCPU() const { return *((CPU6809*)m_cpu); }

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitROM();
		void InitRAM();
		void InitIO();
		void InitVideo();
		void InitLightpen();

		// pia::EventHandler
		virtual void OnScreenMapChange(pia::ScreenRAM map) override;
		virtual void OnBorderChange(BYTE borderRGBP) override;

		video::VideoThomson& GetVideo() { return static_cast<video::VideoThomson&>(*m_video); }

		emul::MemoryBlock m_pixelRAM;
		emul::MemoryBlock m_attributeRAM;

		emul::MemoryBlock m_userRAM;
		emul::MemoryBlock m_osROM;
		emul::MemoryBlock m_basicROM;

		emul::IOBlock m_ioA7C0;

		pia::Device6520MO5_PIA m_pia;
		kbd::DeviceKeyboardThomson m_keyboard;
		mouse::DeviceLightpenThomson m_lightpen;
	};
}
