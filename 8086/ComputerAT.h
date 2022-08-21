#pragma once

#include "Computer.h"
#include "Sound/DevicePCSpeaker.h"
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardAT.h"
#include "Hardware/Device146818.h"
#include "Hardware/DevicePOSTCard.h"
#include "Sound/DeviceGameBlaster.h"
#include "Sound/DeviceSoundSource.h"

namespace emul
{

	class ComputerAT : public Computer
	{
	public:
		ComputerAT();

		virtual std::string_view GetName() const override { return "IBM AT"; };
		virtual std::string_view GetID() const override { return "at"; };
	
		virtual void Reset() override;
		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		virtual kbd::DeviceKeyboard& GetKeyboard() override { return m_keyboard; }

	protected:
		void InitRAM(emul::WORD baseRAM);

		void WriteNMIMask(BYTE value);
		bool m_nmiEnabled = false;

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_highRAM;
		emul::MemoryBlock m_biosF000;

		kbd::DeviceKeyboardAT m_keyboard;

		pic::Device8259 m_picSecondary;

		rtc::Device146818 m_rtc;

		post::DevicePOSTCard m_post;

		cms::DeviceGameBlaster m_gameBlaster;
		bool isSoundGameBlaster = false;

		dss::DeviceSoundSource m_soundDSS;
		bool isSoundDSS = false;

		BYTE ReadHDDStatus() { return 0; }
		void WriteHDDReg(BYTE value) { m_tempHDD = value; }
		BYTE ReadHDDReg() { return m_tempHDD; }
		BYTE m_tempHDD = 0;
	};
}
