#pragma once

#include "MonitorBase.h"
#include <IO/Console.h>
#include <CPU/Memory.h>
#include <CPU/CPUInfo.h>
#include "CPU/CPU6809.h"

namespace emul
{
	class Monitor6809 : public MonitorBase
	{
	public:
		Monitor6809(Console& console);
		virtual ~Monitor6809() {}

		Monitor6809() = delete;
		Monitor6809(const Monitor6809&) = delete;
		Monitor6809& operator=(const Monitor6809&) = delete;
		Monitor6809(Monitor6809&&) = delete;
		Monitor6809& operator=(Monitor6809&&) = delete;

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

		virtual ADDRESS Disassemble(ADDRESS address, Monitor6809::Instruction& decoded);
		void DecodeIndexedInstruction(cpuInfo::Opcode& opcode, BYTE idx);
		void DecodeStackRegs(char* buf, BYTE regs, bool isU);

		enum class RAMMode { DP, X, Y, SP, USP, PC, CUSTOM };
		RAMMode m_ramMode = RAMMode::DP;

		CPU6809* GetCPU() const { return dynamic_cast<CPU6809*>(m_cpu); }
	};
}
