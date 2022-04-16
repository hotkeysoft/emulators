#pragma once

#include "Computer.h"
#include "Sound/DevicePCSpeaker.h"
#include "Sound/DeviceSN76489.h"
#include "Sound/DeviceGameBlaster.h"
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardXT.h"

namespace emul
{
	class ComputerXT : public Computer
	{
	public:
		ComputerXT();

		virtual std::string_view GetName() const override { return "IBM XT"; };
		virtual std::string_view GetID() const override { return "xt"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		virtual kbd::DeviceKeyboard& GetKeyboard() override { return m_keyboard; }

	protected:
		void InitRAM(emul::WORD baseRAM);

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_biosF000;
		emul::MemoryBlock m_biosF800;

		sn76489::DeviceSN76489 m_soundPCjr;
		bool isSoundPCjr = false;

		cms::DeviceGameBlaster m_gameBlaster;
		bool isSoundGameBlaster = false;

		kbd::DeviceKeyboardXT m_keyboard;
	};
}
