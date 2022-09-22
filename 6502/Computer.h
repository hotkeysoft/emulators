#pragma once

#include "CPU/CPU6502.h"
#include <CPU/Memory.h>

#include <Serializable.h>

#include <set>

using emul::WORD;

namespace events { class InputEvents; }

namespace emul
{
	class Computer : public Serializable, public PortConnector
	{
	public:
		const char* CPUID_6502 = "6502";

		virtual ~Computer();

		virtual std::string_view GetName() const = 0;
		virtual std::string_view GetID() const = 0;

		virtual void Init(WORD baseRAM) = 0;

		virtual bool Step() { return m_cpu->Step(); }

		virtual void Reset() { m_cpu->Reset(); }

		bool LoadBinary(const char* file, ADDRESS baseAddress) { return m_memory.LoadBinary(file, baseAddress); }

		CPU6502& GetCPU() const { return *m_cpu; }
		Memory& GetMemory() { return m_memory; }
		events::InputEvents& GetInputs() { return *m_inputs; }

		virtual void Reboot();
		void SetTurbo(bool turbo) { m_turbo = turbo; }

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		Computer(Memory& memory);

		virtual void Init(const char* cpuid, WORD baseRAM);
		virtual void InitInputs(size_t clockSpeedHz, size_t pollingHz = 60);

		Memory m_memory;

		emul::CPU6502* m_cpu = nullptr;
		events::InputEvents* m_inputs = nullptr;

		bool m_turbo = false;
	};
}
