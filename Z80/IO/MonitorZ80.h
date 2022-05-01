#pragma once

#include "Console.h"
#include "Monitor8080.h"
#include "../CPU/Memory.h"
#include "../CPU/CPUInfo.h"

namespace emul
{
	class CPU8080;
	class CPUZ80;

	class MonitorZ80 : public Monitor8080
	{
	public:
		MonitorZ80(Console& console);

		MonitorZ80() = delete;
		MonitorZ80(const Monitor8080&) = delete;
		MonitorZ80& operator=(const Monitor8080&) = delete;
		MonitorZ80(Monitor8080&&) = delete;
		MonitorZ80& operator=(Monitor8080&&) = delete;

		virtual void Init(CPU8080& cpu, Memory& memory) override;

	protected:
		virtual void ToggleRAMMode() override;
		virtual void UpdateRAMMode() override;
		virtual ADDRESS GetRAMBase() const;

		virtual void UpdateRegisters() override;
		virtual ADDRESS Disassemble(ADDRESS address, Monitor8080::Instruction& decoded) override;

		enum class RAMModeZ80 { IX = 10, IY };

		CPUZ80* m_cpuZ80 = nullptr;
	};
}
