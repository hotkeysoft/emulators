#pragma once

#include "Computer.h"
#include "Video/VideoZX80.h"
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardZX80.h"

namespace emul
{
	class CPUZ80;

	class ComputerZX80 : public Computer
	{
	public:
		ComputerZX80();

		virtual std::string_view GetName() const override { return "ZX80"; };
		virtual std::string_view GetID() const override { return "zx80"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		CPUZ80& GetCPU() const { return *((CPUZ80*)m_cpu); }
		video::VideoZX80& GetVideo() { return *((video::VideoZX80*)m_video); }

	protected:
		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_rom;

		void EndVSync(BYTE);
		BYTE ReadKeyboard();

		kbd::DeviceKeyboardZX80 m_keyboard;

	};
}
