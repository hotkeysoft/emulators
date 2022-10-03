#pragma once

#include "Computer.h"
#include "IO/InputEvents.h"
#include <CPU/IOBlock.h>
#include "Hardware/Device6522PET.h"
#include "Hardware/Device6520.h"
#include "Hardware/Device6520PET_PIA1.h"
#include "Video/VideoPET2001.h"
#include "IO/DeviceKeyboardPET2001.h"

namespace emul
{
	class CPU6502;

	class ComputerPET2001 : public Computer
	{
	public:
		ComputerPET2001();

		virtual std::string_view GetName() const override { return "PET2001"; };
		virtual std::string_view GetID() const override { return "pet2001"; };

		virtual void Init(WORD baseRAM) override;
		virtual void Reset() override;
		virtual bool Step() override;

		CPU6502& GetCPU() const { return *((CPU6502*)m_cpu); }

		void SetCassetteSense(int id, bool set);

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		void InitRAM(emul::WORD baseRAM);
		void InitModel();
		void InitROM();
		void InitVideo();
		void InitIO();

		enum class Model { BASIC1, BASIC1p, BASIC2n, BASIC2p };
		Model m_model = Model::BASIC1p;

		const std::string m_basePathROM = "data/PET/PET2001/";
		std::string m_charROM = m_basePathROM;

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_romC000;
		emul::MemoryBlock m_romD000;
		emul::MemoryBlock m_romE000;
		emul::MemoryBlock m_romF000;

		emul::MemoryBlock m_videoRAM;

		emul::IOBlock m_ioE800;

		pia::Device6520PET_PIA1 m_pia1;
		pia::Device6520 m_pia2;
		via::Device6522PET m_via;

		kbd::DeviceKeyboardPET2001 m_keyboard;
	};
}
