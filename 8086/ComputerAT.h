#pragma once

#include "Computer.h"
#include "Sound/DevicePCSpeaker.h"
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardXT.h"
#include "Hardware/Device146818.h"

namespace emul
{
	class POSTCard : public PortConnector
	{
	public:
		POSTCard() : Logger("POST")
		{
		}

		void Init()
		{
			Connect(0x80, static_cast<PortConnector::OUTFunction>(&POSTCard::WritePOST));
		}

		void WritePOST(BYTE value)
		{
			LogPrintf(LOG_INFO, "[%02x]", value);
		}
	};

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
		emul::MemoryBlock m_biosF000;

		kbd::DeviceKeyboardXT m_keyboard;

		pic::Device8259 m_picSecondary;

		dma::Device8237 m_dmaSecondary;

		rtc::Device146818 m_rtc;

		POSTCard m_post;
	};
}
