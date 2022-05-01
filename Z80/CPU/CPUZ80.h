#pragma once
#include "CPU8080.h"

namespace emul
{
	class CPUZ80 : public CPU8080
	{
	public:
		CPUZ80(Memory& memory, Interrupts& interrupts);

		virtual void Init() override;

		void InitIY();

		void InitIX();

		void InitEXTD();

		void InitBITS();

		virtual void Reset() override;

		virtual const std::string GetID() const override { return "z80"; };

	protected:
		CPUZ80(cpuInfo::CPUType type, Memory& memory, Interrupts& interrupts);

		Registers m_regAlt;
		WORD m_regIX = 0;
		WORD m_regIY = 0;

		// Interrupt vector
		BYTE m_regI = 0;

		// Refresh counter
		BYTE m_regR = 0;

		// Interrupt flip-flops
		bool m_iff1 = false;
		bool m_iff2 = false;

		enum InterruptMode {IM0 = 0, IM1, IM2 } m_interruptMode = InterruptMode::IM0;

		// Interrupt mode

		// Helper functions
		void jumpRelIF(bool condition, BYTE offset);
		void exec(OpcodeTable& table, BYTE opcode);
		
		void loadImm8toIdx(WORD base);

		void NotImplemented(const char* opStr);

		// Opcodes
		void EXAF();
		void DJNZ();
		void EXX();

		void BITS(BYTE op2);
		void EXTD(BYTE op2);
		void IX(BYTE op2);
		void IY(BYTE op2);

		OpcodeTable m_opcodesBITS;
		OpcodeTable m_opcodesEXTD;
		OpcodeTable m_opcodesIX;
		OpcodeTable m_opcodesIY;

		friend class MonitorZ80;
	};
}
