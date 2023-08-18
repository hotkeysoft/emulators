#pragma once

#include <Computer/ComputerBase.h>
#include <CPU/IOBlock.h>
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardThomson.h"
#include "IO/DeviceLightpenThomson.h"
#include "Hardware/PIAEventsThomson.h"
#include "Video/VideoThomson.h"
#include "ThomsonModel.h"
#include <Storage/CartridgeLoader.h>
#include <Storage/DeviceTape.h>

namespace pia::thomson { class DevicePIAThomson; }
using ThomsonModel = emul::Thomson::Model;

namespace emul
{
	class CPU6809;

	class ComputerThomson :
		public ComputerBase,
		public IOConnector,
		public pia::thomson::EventHandler,
		public CartridgeLoader
	{
	public:
		ComputerThomson();
		virtual ~ComputerThomson();

		virtual std::string_view GetName() const override { return "Thomson"; };
		virtual std::string_view GetID() const override { return "thomson"; };

		virtual std::string_view GetModel() const override
		{
			static const std::string model = StringUtil::ToUpper(emul::Thomson::ModelToString(m_model));
			return model;
		}

		virtual void Init(WORD baseRAM) override;
		void InitCartridge();
		virtual void Reset() override;
		virtual bool Step() override;

		CPU6809& GetCPU() const { return *((CPU6809*)m_cpu); }

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitModel();
		void InitROM();
		void InitRAM(WORD baseRAM);
		void InitIO();
		void InitKeyboard();
		void InitVideo();
		void InitLightpen();
		void InitTape();

		// Shared constants
		static constexpr DWORD   CPU_CLOCK = 1000000;
		static constexpr DWORD   INPUT_REFRESH_HZ = 50;
		static constexpr DWORD   INPUT_REFRESH_PERIOD = CPU_CLOCK / INPUT_REFRESH_HZ;

		static constexpr WORD    IO_BLOCK_SIZE          = 64;
		static constexpr WORD    RAM_SCREEN_SIZE        = 0x2000;
		static constexpr WORD    ROM_CARTRIDGE_SIZE     = 0x4000;

		// TO7 constants
		static constexpr WORD    TO7_RAM_USER_SIZE      = 0x2000;
		static constexpr WORD    TO7_RAM_EXT_SIZE       = 0x4000;
		static constexpr WORD    TO7_ROM_MONITOR_SIZE   = 0x1800;

		static constexpr ADDRESS TO7_IO_BASE            = 0xE7C0;
		static constexpr ADDRESS TO7_ROM_CARTRIDGE_BASE = 0x0000;
		static constexpr ADDRESS TO7_RAM_SCREEN_BASE    = 0x4000;
		static constexpr ADDRESS TO7_RAM_USER_BASE      = 0x6000;
		static constexpr ADDRESS TO7_RAM_EXT_BASE       = 0x8000;
		static constexpr ADDRESS TO7_ROM_MONITOR_BASE   = 0xE800;

		// MO5 constants
		static constexpr WORD    MO5_RAM_USER_SIZE      = 0x8000;
		static constexpr WORD    MO5_ROM_BASIC_SIZE     = 0x3000;
		static constexpr WORD    MO5_ROM_MONITOR_SIZE   = 0x1000;

		static constexpr ADDRESS MO5_IO_BASE            = 0xA7C0;
		static constexpr ADDRESS MO5_ROM_CARTRIDGE_BASE = 0xB000;
		static constexpr ADDRESS MO5_RAM_SCREEN_BASE    = 0x0000;
		static constexpr ADDRESS MO5_RAM_USER_BASE      = 0x2000;
		static constexpr ADDRESS MO5_ROM_BASIC_BASE     = 0xC000;
		static constexpr ADDRESS MO5_ROM_MONITOR_BASE   = 0xF000;

		// CartridgeLoader
		virtual void LoadCartridge(const std::filesystem::path& path) override;
		virtual hscommon::fileUtil::SelectFileFilters GetLoadFilter() override;

		virtual void UnloadCartridge() override;
		virtual std::string GetCartridgeInfo() const override { return m_cartridgeInfo; }
		std::string m_cartridgeInfo;

		ThomsonModel m_model = ThomsonModel::MO5;

		const std::string m_basePathROM = "data/Thomson/";

		// pia::EventHandler
		virtual void OnScreenMapChange(pia::thomson::ScreenRAM map) override;
		virtual void OnBorderChange(BYTE borderRGBP) override;

		virtual tape::DeviceTape* GetTape() override { return m_tape; }
		video::VideoThomson& GetVideo() { return static_cast<video::VideoThomson&>(*m_video); }

		emul::MemoryBlock m_pixelRAM;
		emul::MemoryBlock m_attributeRAM;
		ADDRESS m_screenRAMBase = 0;

		emul::MemoryBlock m_userRAM;
		emul::MemoryBlock m_extRAM;
		emul::MemoryBlock m_osROM;
		emul::MemoryBlock m_basicROM;
		emul::MemoryBlock m_cartridgeROM;

		emul::IOBlock m_io;

		pia::thomson::DevicePIAThomson* m_pia = nullptr;
		kbd::DeviceKeyboardThomson m_keyboard;
		mouse::DeviceLightpenThomson m_lightpen;
		tape::DeviceTape* m_tape = nullptr;
	};
}
