#pragma once

#include <Computer/ComputerBase.h>
#include "Video/VideoCPC464.h"
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardZX80.h"

namespace emul
{
	class CPUZ80;

	class ComputerCPC464 : public ComputerBase
	{
	public:
		ComputerCPC464();

		virtual std::string_view GetName() const override { return "CPC464"; };
		virtual std::string_view GetID() const override { return "cpc464"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		CPUZ80& GetCPU() const { return *((CPUZ80*)m_cpu); }
		video::VideoCPC464& GetVideo() { return *((video::VideoCPC464*)m_video); }

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitVideo();

		void NullWrite(BYTE) {}

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_romLow;
		emul::MemoryBlock m_romHigh;

		kbd::DeviceKeyboardZX80 m_keyboard;
	};
}
