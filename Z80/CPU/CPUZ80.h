#pragma once
#include "CPU8080.h"

namespace emul
{
	class CPUZ80 : public CPU8080
	{
	public:
		CPUZ80(Memory& memory, Interrupts& interrupts);

		virtual void Init() override;

		virtual void Reset() override;

		virtual const std::string GetID() const override { return "z80"; };

		virtual void Exec(BYTE opcode) override;
		virtual void Halt() override;

		BYTE GetRefreshCounter() const { return m_regR; }

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

		void InitIXY();
		void InitEXTD();
		void InitBITS();
		void InitBITSxy();

		Registers m_regAlt;
		WORD m_regIX = 0;
		WORD m_regIY = 0;
		WORD* m_currIdx = &m_regIX;
		BYTE m_currOffset = 0;
		BYTE m_regDummy = 0;

		// Interrupt vector
		BYTE m_regI = 0;

		// Refresh counter
		BYTE m_regR = 0;
		// Bit 7 is left untouched
		void REFRESH() { bool b7 = GetBit(m_regR, 7); ++m_regR; SetBit(m_regR, 7, b7); }

		// Interrupt flip-flops
		bool m_iff1 = false;
		bool m_iff2 = false;

		// Interrupt mode
		enum InterruptMode {IM0 = 0, IM1, IM2 } m_interruptMode = InterruptMode::IM0;

		// Helper functions
		BYTE ReadMemIdx(BYTE offset);
		void WriteMemIdx(BYTE offset, BYTE value);
		void MEMop(std::function<void(CPUZ80*, BYTE& dest)> func);

		void jumpRelIF(bool condition, BYTE offset);
		void exec(OpcodeTable& table, BYTE opcode);
		
		void loadImm8toIdx(WORD base);

		void adcHL(WORD src);
		void sbcHL(WORD src);

		virtual void add(BYTE src, bool carry = false) override;
		virtual void sub(BYTE src, bool borrow = false) override;
		virtual BYTE cmp(BYTE src) override;

		virtual void inc(BYTE& reg) override;
		virtual void dec(BYTE& reg) override;

		void NotImplemented(const char* opStr);

		// Opcodes
		void EXAF();
		void EXX();

		void BITS(BYTE op2);
		void BITSxy();
		void EXTD(BYTE op2);
		void IXY(BYTE op2);

		OpcodeTable m_opcodesBITS;
		OpcodeTable m_opcodesBITSxy;
		OpcodeTable m_opcodesEXTD;
		OpcodeTable m_opcodesIXY;

		void RL(BYTE& dest);
		void RLC(BYTE& dest);

		void BITget(BYTE bit, BYTE src);
		void BITgetIXY(BYTE bit);
		void BITset(BYTE bit, bool set, BYTE& dest);
		void BITsetIXY(BYTE bit, bool set, BYTE& altDest);
		void BITsetM(BYTE bit, bool set);

		void INc(BYTE& dest);

		friend class MonitorZ80;
	};
}
