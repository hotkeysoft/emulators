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

		enum FLAGZ80 : BYTE
		{
			FLAG_S = 128, // Sign
			FLAG_Z = 64,  // Zero
			FLAG_F5 = 32, // undocumented flag - Copy of bit 5
			FLAG_H = 16,  // Half Carry 
			FLAG_F3 = 8,  // undocumented flag - Copy of bit 3
			FLAG_PV = 4,  // Parity/Overflow
			FLAG_N = 2,   // Subtract - Set if the last operation was a subtraction
			FLAG_CY = 1   // Carry
		};

		bool GetFlag(FLAGZ80 f) { return (m_reg.flags & f) ? true : false; };
		void SetFlag(FLAGZ80 f, bool v) { SetBitMask(m_reg.flags, f, v); };
		virtual void AdjustBaseFlags(BYTE val) override;
		void AdjustBaseFlags(WORD val);

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

		// Interrupt mode
		enum InterruptMode {IM0 = 0, IM1, IM2 } m_interruptMode = InterruptMode::IM0;

		// Helper functions
		BYTE ReadMemIdx(WORD base);
		void MEMop(std::function<void(CPUZ80*, BYTE& dest)> func);

		void jumpRelIF(bool condition, BYTE offset);
		void exec(OpcodeTable& table, BYTE opcode);
		
		void loadImm8toIdx(WORD base);

		void adcHL(WORD src);
		void sbcHL(WORD src);

		void NotImplemented(const char* opStr);

		// Opcodes
		void EXAF();
		void EXX();

		void BITS(BYTE op2);
		void EXTD(BYTE op2);
		void IX(BYTE op2);
		void IY(BYTE op2);

		OpcodeTable m_opcodesBITS;
		OpcodeTable m_opcodesEXTD;
		OpcodeTable m_opcodesIX;
		OpcodeTable m_opcodesIY;

		void RL(BYTE& dest);
		void BITget(BYTE bit, BYTE src);
		void BITset(BYTE bit, bool set, BYTE& dest);
		void BITsetM(BYTE bit, bool set);

		friend class MonitorZ80;
	};
}
