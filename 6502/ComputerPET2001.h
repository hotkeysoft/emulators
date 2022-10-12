#pragma once

#include "Computer/ComputerBase.h"
#include "IO/InputEvents.h"
#include <CPU/IOBlock.h>
#include "Hardware/Device6522PET.h"
#include "Hardware/Device6520.h"
#include "Hardware/Device6520PET_PIA1.h"
#include "Video/VideoPET2001.h"
#include <Storage/DeviceTape.h>

namespace kbd { class DeviceKeyboardPET2001; }

namespace emul
{
	class CPU6502;

	class ComputerPET2001 : public ComputerBase
	{
	public:
		enum class Model { UNKNOWN, BASIC1, BASIC1p, BASIC2n, BASIC2b, BASIC4n, BASIC4b };

		ComputerPET2001();
		virtual ~ComputerPET2001();

		virtual std::string_view GetName() const override { return "PET2001"; };
		virtual std::string_view GetID() const override { return "pet2001"; };

		virtual void Init(WORD baseRAM) override;
		virtual void Reset() override;
		virtual bool Step() override;

		CPU6502& GetCPU() const { return *((CPU6502*)m_cpu); }

		Model GetModel() const { return m_model; }
		static Model StringToModel(const char*);
		static std::string ModelToString(Model);

		virtual tape::DeviceTape* GetTape() override { return &m_tape; }

		void LoadPRG(const char* file);

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitModel();
		void InitKeyboard();
		void InitRAM(emul::WORD baseRAM);
		void InitROM();
		void InitIO();
		void InitVideo();
		void InitTape();

		std::string GetCharROMPath();

		static const std::map<std::string, Model> s_modelMap;
		Model m_model = Model::BASIC1p;

		const std::string m_basePathROM = "data/PET/PET2001/";
		const std::string m_basePathROM4 = "data/PET/PET4000/";

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_romB000;
		emul::MemoryBlock m_romC000;
		emul::MemoryBlock m_romD000;
		emul::MemoryBlock m_romE000;
		emul::MemoryBlock m_romF000;

		emul::MemoryBlock m_videoRAM;

		emul::IOBlock m_ioE800;

		pia::Device6520PET_PIA1 m_pia1;
		pia::Device6520 m_pia2;
		via::Device6522PET m_via;

		kbd::DeviceKeyboardPET2001* m_keyboard = nullptr;

		tape::DeviceTape m_tape;
	};
}
