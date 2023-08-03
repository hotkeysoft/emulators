#pragma once

#include <Computer/ComputerBase.h>
#include "Video/VideoCPC464.h"
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardCPC464.h"
#include "IO/DeviceJoystickDigital.h"
#include "Hardware/Device8255CPC464.h"
#include "Sound/DeviceAY-3-891x.h"
#include <Storage/DeviceTape.h>

namespace vid464 = video::cpc464;
namespace fdc { class DeviceFloppy; }

namespace emul
{
	class CPUZ80;

	class ComputerCPC464 : public ComputerBase, public vid464::EventHandler
	{
	public:
		enum class Model { UNKNOWN, CPC464, CPC664, CPC6128 };

		ComputerCPC464();
		~ComputerCPC464();

		virtual std::string_view GetName() const override { return "CPC"; };
		virtual std::string_view GetID() const override { return "cpc"; };
		virtual std::string_view GetModel() const override
		{
			static const std::string model = ModelToString(m_model);
			return model;
		}

		virtual void Reset() override;

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		CPUZ80& GetCPU() const { return *((CPUZ80*)m_cpu); }

		static Model StringToModel(const char*);
		static std::string ModelToString(Model);

		fdc::DeviceFloppy* GetFloppy() { return m_floppy; }
		virtual tape::DeviceTape* GetTape() override { return m_tape; }
		vid464::VideoCPC464& GetVideo() { return *((vid464::VideoCPC464*)m_video); }

		// vid464::EventHandler
		virtual void OnLowROMChange(bool load) override;
		virtual void OnHighROMChange(bool load) override;

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitModel();
		void InitKeyboard();
		void InitJoystick();
		void InitRAM();
		void InitROM();
		void InitIO();
		void InitVideo();
		void InitSound();
		void InitTape();
		void InitFloppy(fdc::DeviceFloppy* fdd);

		static const std::map<std::string, Model> s_modelMap;
		Model m_model = Model::CPC464;

		const std::string m_basePathROM = "data/z80/amstrad/";

		void LoadROM(bool load, emul::MemoryBlock* rom, ADDRESS base);
		bool m_highROMLoaded = false;

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_romLow;

		static const ADDRESS ROM_LOW = 0;
		static const ADDRESS ROM_HIGH = 0xC000;
		static const ADDRESS ROM_SIZE = 0x4000;
		static const size_t ROM_BANKS = 256;

		std::array<emul::MemoryBlock*, ROM_BANKS> m_romBanks = { nullptr };

		BYTE m_currHighROM = 0;
		emul::MemoryBlock* GetCurrHighROM() const { return m_romBanks[m_currHighROM] ? m_romBanks[m_currHighROM] : m_romBanks[0]; }
		bool LoadHighROM(BYTE bank, const char* romFile);
		void SelectROMBank(BYTE value);

		void TickFloppy();

		kbd::DeviceKeyboardCPC464 m_keyboard;
		ppi::Device8255CPC464 m_pio;
		tape::DeviceTape* m_tape = nullptr;
		fdc::DeviceFloppy* m_floppy = nullptr;
		joy::DeviceJoystickDigital m_joystick;
		sound::ay3::DeviceAY_3_891x m_sound;
	};
}
