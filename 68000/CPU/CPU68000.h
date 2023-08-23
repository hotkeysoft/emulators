#pragma once

#include <Serializable.h>
#include <CPU/CPU.h>
#include <CPU/PortConnector.h>
#include <CPU/CPUInfo.h>
#include <EdgeDetectLatch.h>

#undef IN
#undef OUT

namespace emul
{
	static const size_t CPU68000_ADDRESS_BITS = 24;
	static const char* CPUID_68000 = "68000";

	class CPU68000 : public CPU, public PortConnector
	{
	public:
		CPU68000(Memory& memory);
		virtual ~CPU68000();

		virtual void Init();

		void Dump();

		virtual void Reset();
		virtual void Reset(ADDRESS overrideAddress);

		virtual bool Step() override;
		// Not used, 16 bit opcode
		virtual void Exec(BYTE opcode) override;

		virtual const std::string GetID() const override { return m_info.GetId(); };
		virtual size_t GetAddressBits() const override { return CPU68000_ADDRESS_BITS; };
		virtual ADDRESS GetCurrentAddress() const override { return m_programCounter; }

		const cpuInfo::CPUInfo& GetInfo() const { return m_info; }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:

		CPU68000(const char* cpuid, Memory& memory);

		inline void TICK() { m_opTicks += (*m_currTiming)[(int)cpuInfo::OpcodeTimingType::BASE]; };

		// Hardware vectors
		enum class VECTOR : BYTE
		{
			ResetSSP               = 0, // Reset, Initial SSP (Supervisor Stack Pointer)
			ResetPC                = 1, // Reset, Initial PC (Program Counter)
			BusError               = 2,
			AddressError           = 3,
			IllegalInstruction     = 4,
			ZeroDivide             = 5,
			CHK_Instruction        = 6,
			TRAPV_Instruction      = 7,
			PrivilegeViolation     = 8,
			Trace                  = 9,
			Line1010Emulator       = 10,
			Line1111Emulator       = 11,

			// (12-14 Reserved)

			UninitializedIntVector = 15,

			// (16-23 Reserved)

			SpuriousInterrupt      = 24,

			// 25-31 Level 1-7 Interrupt Autovector
			InterruptBase      = 25,

			// 32-47 TRAP Instructions vectors (32 + n)
			TrapBase              = 32,

			// 48-63 Reserved
			// 64-255 User Interrupt Vectors
			USER_INT_BASE     = 64
		};

		ADDRESS GetVectorAddress(VECTOR v) { return (ADDRESS)v * 4; }
		ADDRESS GetIntVectorAddress(BYTE i) { assert(i <= 7); return ((ADDRESS)VECTOR::InterruptBase + i) * 4; }
		ADDRESS GetTrapVectorAddress(BYTE t) { assert(t <= 16);  return ((ADDRESS)VECTOR::TrapBase + t) * 4; }

		using OpcodeTable = std::vector<std::function<void()>>;
		OpcodeTable m_opcodes;
		void UnknownOpcode();

		cpuInfo::CPUInfo m_info;
		const cpuInfo::OpcodeTiming* m_currTiming = nullptr;
		WORD m_opcode = 0;

		virtual void Interrupt();

		enum FLAG : WORD
		{
			// Supervisor mode flags (R/O in user mode)
			FLAG_T		= 0x8000, // 1 Trace mode
			FLAG_R14	= 0x4000, // 0
			FLAG_S		= 0x2000, // 1 Supervisor mode
			_FLAG_R12	= 0x1000, // 0
			_FLAG_R11	= 0x0800, // 0
			FLAG_I2 	= 0x0400, // I[0..2], interrupt mask
			FLAG_I1		= 0x0200, // ""
			FLAG_I0		= 0x0100, // ""

			// User mode R/W
			_FLAG_R7	= 0x0080, // 0
			_FLAG_R6	= 0x0040, // 0
			_FLAG_R5	= 0x0020, // 0
			FLAG_X		= 0x0010, // 1 eXtend
			FLAG_N      = 0x0008, // 1 Negative
			FLAG_Z		= 0x0004, // 1 Zero
			FLAG_V		= 0x0002, // 1 Signed oVerflow
			FLAG_C		= 0x0001  // 1 Carry (unsigned overflow)
		};

		FLAG FLAG_RESERVED_OFF = FLAG(
			_FLAG_R5 | _FLAG_R6 | _FLAG_R7 |
			_FLAG_R11 | FLAG_R14);

		ADDRESS m_programCounter = 0;

		struct Registers
		{
			std::array<DWORD, 8> DATA; // D0..D7
			std::array<DWORD, 8> ADDR; // A0..A7

			// Alias stack pointer to A7
			DWORD& SP = ADDR[7];

			// Stack pointer is aliased to A7 but user and supervisor
			// modes keep separate values

			// USP (User) Stack Pointer
			DWORD USP;
			// SSP (Supervisor) Stack Pointer
			DWORD SSP;

			WORD flags;
		} m_reg;

		void ClearFlags(WORD& flags);
		void SetFlags(WORD f);

		// Flag Helpers
		bool GetFlag(FLAG f) const { return (m_reg.flags & f) ? true : false; };
		void SetFlag(FLAG f, bool v) { SetBitMask(m_reg.flags, f, v); };
		void ComplementFlag(FLAG f) { m_reg.flags ^= f; }

		bool IsSupervisorMode() const { return GetFlag(FLAG_S); }
		void SetSupervisorMode(bool set);

		bool IsTrace() const { return GetFlag(FLAG_T); }
		void SetTrace(bool set) { SetFlag(FLAG_T, set); }

		BYTE GetInterruptMask() const { return (m_reg.flags >> 8) | 7; }
		void SetInterruptMask(BYTE mask) {
			assert(mask < 8);
			SetFlag(FLAG_I0, GetBit(mask, 0));
			SetFlag(FLAG_I1, GetBit(mask, 1));
			SetFlag(FLAG_I2, GetBit(mask, 2));
		}

		// Not used, 16 bit fetch
		virtual BYTE FetchByte() override;

		ADDRESS GetSP() const { return m_reg.SP; }
		void Exec(WORD opcode);
		bool InternalStep();

		friend class Monitor68000;
	};
}
