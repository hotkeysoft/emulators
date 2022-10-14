#pragma once

#include "Computer/ComputerBase.h"
#include "IO/InputEvents.h"
#include <CPU/IOBlock.h>
#include "Hardware/Device6522.h"
#include <Storage/DeviceTape.h>

namespace kbd { class DeviceKeyboardPET2001; }

namespace emul
{
	class CPU6502;

	class ComputerVIC20 : public ComputerBase
	{
	public:
		ComputerVIC20();
		virtual ~ComputerVIC20();

		virtual std::string_view GetName() const override { return "VIC20"; };
		virtual std::string_view GetID() const override { return "vic20"; };

		virtual void Init(WORD baseRAM) override;
		virtual void Reset() override;
		virtual bool Step() override;

		CPU6502& GetCPU() const { return *((CPU6502*)m_cpu); }

		virtual tape::DeviceTape* GetTape() override { return &m_tape; }

		void LoadPRG(const char* file);

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitKeyboard();
		void InitRAM(emul::WORD baseRAM);
		void InitROM();
		void InitIO();
		void InitVideo();
		void InitTape();

		emul::MemoryBlock m_ramBlock0LOW;  // 1K Low memory
		emul::MemoryBlock m_ramBlock0RAM1; // 1K (expansion /RAM1)
		emul::MemoryBlock m_ramBlock0RAM2; // 1K (expansion /RAM2)
		emul::MemoryBlock m_ramBlock0RAM3; // 1K (expansion /RAM3)
		emul::MemoryBlock m_ramBlock0MAIN; // 4K Main RAM

		emul::MemoryBlock m_ramBlock1; // 8K (expansion /BLK1)
		emul::MemoryBlock m_ramBlock2; // 8K (expansion /BLK2)
		emul::MemoryBlock m_ramBlock3; // 8K (expansion /BLK3)

		// Block4, char ROM, IO, Color Ram, expansion IO
		emul::MemoryBlock m_romCHAR; // 0x8000

		// Due to incomplete decoding, there can be device overlap if you don't use
		// the "legal" io addresses. This is not really emulated here.
		// Probably undefined behavior
		emul::IOBlock m_ioVIC; // 0x9000-90FF
		emul::IOBlock m_ioVIA; // 0x9100-93FF

		emul::MemoryBlock m_ramCOLOR; // 1K (nibbles), 0x9400 or 0x9600 depending on VIC register

		// IO2, 3, not implemented

		emul::MemoryBlock m_ramBlock5; // 8K (expansion /BLK5), autostart

		emul::MemoryBlock m_romBASIC;
		emul::MemoryBlock m_romKERNAL;

		via::Device6522 m_via1;
		via::Device6522 m_via2;

		kbd::DeviceKeyboardPET2001* m_keyboard = nullptr;

		tape::DeviceTape m_tape;
	};
}
