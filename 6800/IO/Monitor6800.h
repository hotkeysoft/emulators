#pragma once

#include "MonitorBase.h"
#include <IO/Console.h>
#include <CPU/Memory.h>
#include <CPU/CPUInfo.h>
#include "CPU/CPU6800.h"

namespace emul
{
	class Monitor6800 : public MonitorBase
	{
	public:
		Monitor6800(Console& console);
		virtual ~Monitor6800() {}

		Monitor6800() = delete;
		Monitor6800(const Monitor6800&) = delete;
		Monitor6800& operator=(const Monitor6800&) = delete;
		Monitor6800(Monitor6800&&) = delete;
		Monitor6800& operator=(Monitor6800&&) = delete;

		virtual void Init(CPU* cpu, Memory& memory) override;

		virtual void Show() override;
		virtual MonitorState Run() override;
		virtual void Update() override;

	protected:
		struct Instruction
		{
			void AddRaw(BYTE b);
			void AddRaw(WORD w);

			ADDRESS address;
			BYTE len = 0;
			BYTE raw[16];
			char text[32];
		};

		MonitorState ProcessKey();

		void ToggleRunMode();
		void UpdateRunMode();
		virtual void ToggleRAMMode();
		virtual void UpdateRAMMode();
		void UpdateCPUType();

		virtual void UpdateRegisters();
		void UpdateTicks();
		void UpdateFlags();
		virtual ADDRESS GetRAMBase() const;
		void UpdateRAM();
		void PrintInstruction(short y, Instruction& instr);
		void UpdateCode();
		void ClearCode();

		virtual ADDRESS Disassemble(ADDRESS address, Monitor6800::Instruction& decoded);

		enum class RAMMode { ZP, IX, SP, PC, CUSTOM };
		RAMMode m_ramMode = RAMMode::IX;

		CPU6800* GetCPU() const { return dynamic_cast<CPU6800*>(m_cpu); }
	};
}
