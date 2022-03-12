#pragma once

#include "Computer.h"
#include "Hardware/Device8250.h"
#include "Sound/DeviceSN76489.h"
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardTandy.h"

using emul::WORD;

namespace emul
{
	class DummyPortTandy : public PortConnector
	{
	public:
		DummyPortTandy() : Logger("DUMMY")
		{
			//EGA
			for (WORD w = 0x3C0; w < 0x3D0; ++w)
			{
				Connect(w, static_cast<PortConnector::OUTFunction>(&DummyPortTandy::WriteData));
			}

			// MPU-401
			Connect(0x330, static_cast<PortConnector::OUTFunction>(&DummyPortTandy::WriteData));
			Connect(0x330, static_cast<PortConnector::INFunction>(&DummyPortTandy::ReadData));
			Connect(0x331, static_cast<PortConnector::OUTFunction>(&DummyPortTandy::WriteData));
			Connect(0x331, static_cast<PortConnector::INFunction>(&DummyPortTandy::ReadData));
		}

		BYTE ReadData()
		{
			return 0xFF;
		}

		void WriteData(BYTE value)
		{
		}
	};

	class ComputerTandy : public Computer
	{
	public:
		ComputerTandy();

		virtual std::string_view GetName() const override { return "Tandy 1000"; };
		virtual std::string_view GetID() const override { return "tandy"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		Memory& GetMemory() { return m_memory; }
		virtual kbd::DeviceKeyboard& GetKeyboard() override { return m_keyboard; }

	protected:
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

		void SetRAMPage(BYTE value);
		void InitRAM(WORD baseRAM);

		void SetRAMBase(ADDRESS ramBase);
		ADDRESS GetRAMBase() const { return m_ramBase; }

		emul::MemoryBlock m_base128K;
		emul::MemoryBlock m_ramExtension;
		emul::MemoryBlock m_biosFC00;

		kbd::DeviceKeyboardTandy m_keyboard;
		uart::Device8250 m_uart;
		sn76489::DeviceSN76489 m_soundModule;

		DummyPortTandy m_dummyPorts;

	private:
		ADDRESS m_ramBase = 0;
	};
}
