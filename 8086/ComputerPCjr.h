#pragma once

#include "Computer.h"
#include "Hardware/Device8250.h"
#include "Sound/DeviceSN76489.h"
#include "Storage/CartridgePCjr.h"
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardPCjr.h"

using emul::WORD;

namespace emul
{
	class DummyPortPCjr : public PortConnector
	{
	public:
		DummyPortPCjr() : Logger("DUMMY")
		{
		}

		void Init()
		{
			Connect(0x10, static_cast<PortConnector::OUTFunction>(&DummyPortPCjr::WriteMfgTest));
		}

		BYTE ReadData()
		{
			return 0xFF;
		}

		void WriteMfgTest(BYTE value)
		{
			LogPrintf(LOG_ERROR, "MFG TEST: %02Xh", value);
		}
	};

	class ComputerPCjr : public Computer
	{
	public:
		ComputerPCjr();

		virtual std::string_view GetName() const override { return "IBM PCjr"; };
		virtual std::string_view GetID() const override { return "pcjr"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		virtual kbd::DeviceKeyboard& GetKeyboard() override { return m_keyboard; }

	protected:
		void InitRAM(WORD baseRAM);

		virtual void TickFloppy() override;

		emul::MemoryBlock m_base64K;
		emul::MemoryBlock m_ext64K;
		emul::MemoryBlock m_extraRAM;

		emul::MemoryBlock m_biosF000;
		emul::MemoryBlock m_biosF800;

		cart::CartridgePCjr m_cart1;
		cart::CartridgePCjr m_cart2;

		kbd::DeviceKeyboardPCjr m_keyboard;
		uart::Device8250 m_uart;
		sn76489::DeviceSN76489 m_soundModule;

		DummyPortPCjr m_dummyPorts;
	};
}
