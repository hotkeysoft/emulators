#pragma once
#include "CPU8080.h"
#include "EdgeDetectLatch.h"

namespace emul
{
	class CPUZ80 : public CPU8080
	{
	public:
		CPUZ80(Memory& memory);

		virtual void Init() override;

		virtual void Reset() override;

		virtual const std::string GetID() const override { return "z80"; };

		virtual void Exec(BYTE opcode) override;
		virtual void Halt() override;

		BYTE GetRefreshCounter() const { return m_regR; }

		// Positive logic
		void SetNMI(bool value) { m_nmiLatch.Set(value); }
		// Positive logic
		void SetINT(bool value) { m_intLatch.Set(value); }

		// This is needed only for the ZX80/81: Disconnects the data bus so the CPU sees NOPs
		void EnableDataBus(bool enable) { m_dataBusEnable = enable; }

		// On the Z80, IN/OUT puts reg A or B value in A8..A15
		// This can be used in IO handlers
		BYTE GetIOHighAddress() const { return m_ioHighAddress; }

	protected:
		CPUZ80(cpuInfo::CPUType type, Memory& memory);

		virtual BYTE FetchByte() override { BYTE op = CPU8080::FetchByte();  return m_dataBusEnable ? op : 0; }

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
		void ComplementFlag(FLAGZ80 f) { m_reg.flags ^= f; }
		virtual void AdjustBaseFlags(BYTE val) override;
		void AdjustBaseFlags(WORD val);

		void InitIXY();
		void InitEXTD();
		void InitBITS();
		void InitBITSxy();

		bool m_dataBusEnable = true;
		BYTE m_ioHighAddress = 0;
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
		hscommon::EdgeDetectLatch m_nmiLatch;
		hscommon::EdgeDetectLatch m_intLatch;

		bool m_iff1 = false;
		bool m_iff2 = false;
		virtual void Interrupt() override;

		// Interrupt mode
		enum InterruptMode {IM0 = 0, IM1, IM2 } m_interruptMode = InterruptMode::IM0;

		// Helper functions
		BYTE ReadMemIdx(BYTE offset);
		void WriteMemIdx(BYTE offset, BYTE value);
		void IDXop(std::function<void(CPUZ80*, BYTE& dest)> func);
		void IDXop(std::function<void(CPUZ80*, BYTE& dest)> func, BYTE& reg);
		void MEMop(std::function<void(CPUZ80*, BYTE& dest)> func);

		void jumpRelIF(bool condition, BYTE offset);
		void exec(OpcodeTable& table, BYTE opcode);
		
		void loadImm8toIdx(WORD base);

		BYTE& GetIdxH() { return *(((BYTE*)m_currIdx) + 1); }
		BYTE& GetIdxL() { return *((BYTE*)m_currIdx); }

		void addHL(WORD src);
		void addIXY(WORD src);
		void add16(WORD& dest, WORD src, bool carry = false);

		void adcHL(WORD src);
		void sbcHL(WORD src);

		virtual void add(BYTE src, bool carry = false) override;
		virtual void sub(BYTE src, bool borrow = false) override;
		virtual BYTE cmp(BYTE src) override;

		virtual void inc(BYTE& reg) override;
		virtual void dec(BYTE& reg) override;

		void push(WORD src);
		void pop(WORD& dest);

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

		void LDAri(BYTE src);

		void LDI();
		void LDD();

		void CPI();
		void CPD();

		void INI();
		void IND();

		void OUTI();
		void OUTD();

		void rep(std::function<void(CPUZ80*)> func, bool checkZ = false);
		void repz(std::function<void(CPUZ80*)> func);

		void NEG();

		void RLA() { rl(m_reg.A, false); }
		void RLCA() { rlc(m_reg.A, false); }

		void RRA() { rr(m_reg.A, false); }
		void RRCA() { rrc(m_reg.A, false); }

		void RL(BYTE& dest) { rl(dest); }
		void RLC(BYTE& dest) { rlc(dest); }

		void RR(BYTE& dest) { rr(dest); }
		void RRC(BYTE& dest) { rrc(dest); }

		void rl(BYTE& dest, bool adjustBaseFlags = true);
		void rlc(BYTE& dest, bool adjustBaseFlags = true);

		void rr(BYTE& dest, bool adjustBaseFlags = true);
		void rrc(BYTE& dest, bool adjustBaseFlags = true);

		void RLD();
		void RRD();

		void SLA(BYTE& dest) { sla(dest, false); }
		void SLL(BYTE& dest) { sla(dest, true); }

		void SRA(BYTE& dest) { sra(dest, false); }
		void SRL(BYTE& dest) { sra(dest, true); }

		void sla(BYTE& dest, bool bit0);
		void sra(BYTE& dest, bool clearBit7);

		void BITget(BYTE bit, BYTE src);
		void BITgetIXY(BYTE bit);
		void BITset(BYTE bit, bool set, BYTE& dest);
		void BITsetIXY(BYTE bit, bool set, BYTE& altDest);
		void BITsetM(BYTE bit, bool set);

		void INCXY(BYTE offset);
		void DECXY(BYTE offset);

		void INc(BYTE& dest);
		void OUTc(BYTE& dest);

		virtual void DAA() override;

		virtual void DI() override;
		virtual void EI() override;

		friend class MonitorZ80;
	};
}
