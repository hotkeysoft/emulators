#pragma once

#include "Computer.h"
#include "Sound/DeviceSN76489.h"
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardTandy.h"

using emul::WORD;

namespace emul
{
	class ComputerTandy : public Computer
	{
	public:
		ComputerTandy();

		virtual std::string_view GetName() const override { return "Tandy 1000"; };
		virtual std::string_view GetID() const override { return "tandy"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

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
		sn76489::DeviceSN76489 m_soundModule;

	private:
		ADDRESS m_ramBase = 0;
	};
}
