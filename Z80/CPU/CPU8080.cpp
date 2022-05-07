#include "stdafx.h"
#include "CPU8080.h"

using cpuInfo::Opcode;
using cpuInfo::CPUType;

namespace emul
{
	CPU8080::CPU8080(Memory& memory) : CPU8080(CPUType::i8080, memory)
	{
	}


	CPU8080::CPU8080(cpuInfo::CPUType type, Memory& memory) :
		CPU(CPU8080_ADDRESS_BITS, memory),
		m_info(type),
		Logger("CPU8080")
	{
		try
		{
			m_info.LoadConfig();
		}
		catch (nlohmann::detail::exception e)
		{
			LogPrintf(LOG_ERROR, "Fatal json error loading config: %s", e.what());
			throw;
		}
		catch (std::exception e)
		{
			LogPrintf(LOG_ERROR, "Fatal error loading config: %s", e.what());
			throw;
		}
	}

	void CPU8080::Init()
	{
		m_opcodes.resize(256);
		std::fill(m_opcodes.begin(), m_opcodes.end(), [=]() { UnknownOpcode(); });

		// -------------------
		// 1. Data Transfer Group

		// MOV r1,r2 (Move Register)
		// (r1) <- (r2)

		// Destination B
		m_opcodes[0100] = [=]() { m_reg.B = m_reg.B; }; // B,B
		m_opcodes[0101] = [=]() { m_reg.B = m_reg.C; }; // B,C
		m_opcodes[0102] = [=]() { m_reg.B = m_reg.D; }; // B,D
		m_opcodes[0103] = [=]() { m_reg.B = m_reg.E; }; // B,E
		m_opcodes[0104] = [=]() { m_reg.B = m_reg.H; }; // B,H
		m_opcodes[0105] = [=]() { m_reg.B = m_reg.L; }; // B,L
		m_opcodes[0107] = [=]() { m_reg.B = m_reg.A; }; // B,A

		// Destination C
		m_opcodes[0110] = [=]() { m_reg.C = m_reg.B; }; // C,B
		m_opcodes[0111] = [=]() { m_reg.C = m_reg.C; }; // C,C
		m_opcodes[0112] = [=]() { m_reg.C = m_reg.D; }; // C,D
		m_opcodes[0113] = [=]() { m_reg.C = m_reg.E; }; // C,E
		m_opcodes[0114] = [=]() { m_reg.C = m_reg.H; }; // C,H
		m_opcodes[0115] = [=]() { m_reg.C = m_reg.L; }; // C,L
		m_opcodes[0117] = [=]() { m_reg.C = m_reg.A; }; // C,A

		// Destination D
		m_opcodes[0120] = [=]() { m_reg.D = m_reg.B; }; // D,B
		m_opcodes[0121] = [=]() { m_reg.D = m_reg.C; }; // D,C
		m_opcodes[0122] = [=]() { m_reg.D = m_reg.D; }; // D,D
		m_opcodes[0123] = [=]() { m_reg.D = m_reg.E; }; // D,E
		m_opcodes[0124] = [=]() { m_reg.D = m_reg.H; }; // D,H
		m_opcodes[0125] = [=]() { m_reg.D = m_reg.L; }; // D,L
		m_opcodes[0127] = [=]() { m_reg.D = m_reg.A; }; // D,A

		// Destination E	
		m_opcodes[0130] = [=]() { m_reg.E = m_reg.B; }; // E,B
		m_opcodes[0131] = [=]() { m_reg.E = m_reg.C; }; // E,C
		m_opcodes[0132] = [=]() { m_reg.E = m_reg.D; }; // E,D
		m_opcodes[0133] = [=]() { m_reg.E = m_reg.E; }; // E,E
		m_opcodes[0134] = [=]() { m_reg.E = m_reg.H; }; // E,H
		m_opcodes[0135] = [=]() { m_reg.E = m_reg.L; }; // E,L
		m_opcodes[0137] = [=]() { m_reg.E = m_reg.A; }; // E,A
	
		// Destination H
		m_opcodes[0140] = [=]() { m_reg.H = m_reg.B; }; // H,B
		m_opcodes[0141] = [=]() { m_reg.H = m_reg.C; }; // H,C
		m_opcodes[0142] = [=]() { m_reg.H = m_reg.D; }; // H,D
		m_opcodes[0143] = [=]() { m_reg.H = m_reg.E; }; // H,E
		m_opcodes[0144] = [=]() { m_reg.H = m_reg.H; }; // H,H
		m_opcodes[0145] = [=]() { m_reg.H = m_reg.L; }; // H,L
		m_opcodes[0147] = [=]() { m_reg.H = m_reg.A; }; // H,A

		// Destination L
		m_opcodes[0150] = [=]() { m_reg.L = m_reg.B; }; // L,B
		m_opcodes[0151] = [=]() { m_reg.L = m_reg.C; }; // L,C
		m_opcodes[0152] = [=]() { m_reg.L = m_reg.D; }; // L,D
		m_opcodes[0153] = [=]() { m_reg.L = m_reg.E; }; // L,E
		m_opcodes[0154] = [=]() { m_reg.L = m_reg.H; }; // L,H
		m_opcodes[0155] = [=]() { m_reg.L = m_reg.L; }; // L,L
		m_opcodes[0157] = [=]() { m_reg.L = m_reg.A; }; // L,A

		// Destination A
		m_opcodes[0170] = [=]() { m_reg.A = m_reg.B; }; // A,B
		m_opcodes[0171] = [=]() { m_reg.A = m_reg.C; }; // A,C
		m_opcodes[0172] = [=]() { m_reg.A = m_reg.D; }; // A,D
		m_opcodes[0173] = [=]() { m_reg.A = m_reg.E; }; // A,E
		m_opcodes[0174] = [=]() { m_reg.A = m_reg.H; }; // A,H
		m_opcodes[0175] = [=]() { m_reg.A = m_reg.L; }; // A,L
		m_opcodes[0177] = [=]() { m_reg.A = m_reg.A; }; // A,A

		// MOV r, M (Move from memory)
		// (r) <- ((H)(L))
		m_opcodes[0106] = [=]() { m_reg.B = ReadMem(); }; // B,m
		m_opcodes[0116] = [=]() { m_reg.C = ReadMem(); }; // C,m
		m_opcodes[0126] = [=]() { m_reg.D = ReadMem(); }; // D,m
		m_opcodes[0136] = [=]() { m_reg.E = ReadMem(); }; // E,m
		m_opcodes[0146] = [=]() { m_reg.H = ReadMem(); }; // H,m
		m_opcodes[0156] = [=]() { m_reg.L = ReadMem(); }; // L,m
		m_opcodes[0176] = [=]() { m_reg.A = ReadMem(); }; // A,m

		// MOV M, r (Move to memory)
		// ((H)(L) <- (r)
		m_opcodes[0160] = [=]() { WriteMem(m_reg.B); }; // m,B
		m_opcodes[0161] = [=]() { WriteMem(m_reg.C); }; // m,C
		m_opcodes[0162] = [=]() { WriteMem(m_reg.D); }; // m,D
		m_opcodes[0163] = [=]() { WriteMem(m_reg.E); }; // m,E
		m_opcodes[0164] = [=]() { WriteMem(m_reg.H); }; // m,H
		m_opcodes[0165] = [=]() { WriteMem(m_reg.L); }; // m,L
		m_opcodes[0167] = [=]() { WriteMem(m_reg.A); }; // m,A

		// MVI r, data (Move immediate)
		// (r) <- (byte 2)
		m_opcodes[0006] = [=]() { m_reg.B = FetchByte(); }; // B
		m_opcodes[0016] = [=]() { m_reg.C = FetchByte(); }; // C
		m_opcodes[0026] = [=]() { m_reg.D = FetchByte(); }; // D
		m_opcodes[0036] = [=]() { m_reg.E = FetchByte(); }; // E
		m_opcodes[0046] = [=]() { m_reg.H = FetchByte(); }; // H
		m_opcodes[0056] = [=]() { m_reg.L = FetchByte(); }; // L
		m_opcodes[0076] = [=]() { m_reg.A = FetchByte(); }; // A

		// MVI M, data (Move to memory immediate)
		// ((H)(L) <- (byte 2)
		m_opcodes[0066] = [=]() { WriteMem(FetchByte()); };	// m

		// LXI rp, data 16 (Load register pair immediate)
		// (rh) <- (byte3)
		// (lh) <- (byte2)
		m_opcodes[0001] = [=]() { LXIb(); };  // BC
		m_opcodes[0021] = [=]() { LXId(); };  // DE
		m_opcodes[0041] = [=]() { LXIh(); };  // HL
		m_opcodes[0061] = [=]() { LXIsp(); }; // SP

		// LDA addr (Load Accumulator direct)
		// (A) <- ((byte2)(byte3))
		m_opcodes[0072] = [=]() { LDA(); };

		// STA addr (Store Accumulator direct)
		// ((byte2)(byte3)) <- (A)
		m_opcodes[0062] = [=]() { STA(); };

		// LHLD addr (Load H and L direct)
		// (H)<-((byte2)(byte3))
		// (L)<-((byte2)(byte3)+1)
		m_opcodes[0052] = [=]() { LHLD(); };

		// SHLD addr (Store H and L direct)
		// ((byte2)(byte3))<-(H)
		// ((byte2)(byte3)+1)<-(L)
		m_opcodes[0042] = [=]() { SHLD(); };

		// LDAX rp (Load accumulator indirect)
		// (A) <- ((rp))
		// Note: only register pairs rp=B (B and C) or rp=D (D and E) 
		// may be specified.
		m_opcodes[0012] = [=]() { LDAXb(); };
		m_opcodes[0032] = [=]() { LDAXd(); };

		// STAX rp (Store accumulator indirect)
		// ((rp) <- (A)
		// Note: only register pairs rp=B (B and C) or rp=D (D and E) 
		// may be specified.
		m_opcodes[0002] = [=]() { STAXb(); };
		m_opcodes[0022] = [=]() { STAXd(); };

		// XCHG (Exchange H and L with D and E)
		// (H) <- (D)
		// (L) <- (E)
		m_opcodes[0353] = [=]() { XCHG(); };
	
		// -------------------
		// 2. Arithmetic group

		// Unless indicated otherwise, all instructions in this group
		// affect the Zero, Sign, Parity, Carry and Auxiliary Carry
		// flags according to the standard rules.
		// All substraction operations are performed via two's complement
		// arithmetic and set the carry flag to one to indicate a borrow
		// and clear it to indicate no borrow

		// ADD r (Add Register)
		// (A) <- (A) + (r)
		m_opcodes[0200] = [=]() { add(m_reg.B); };	// B
		m_opcodes[0201] = [=]() { add(m_reg.C); };	// C
		m_opcodes[0202] = [=]() { add(m_reg.D); };	// D
		m_opcodes[0203] = [=]() { add(m_reg.E); };	// E
		m_opcodes[0204] = [=]() { add(m_reg.H); };	// H
		m_opcodes[0205] = [=]() { add(m_reg.L); };	// L
		m_opcodes[0207] = [=]() { add(m_reg.A); };	// A

		// ADD M (Add memory)
		// (A) <- (A) + ((H)(L))
		m_opcodes[0206] = [=]() { add(ReadMem()); }; // m

		// ADI data (Add immediate)
		// (A) <- (A) + (byte2)
		m_opcodes[0306] = [=]() { add(FetchByte()); };

		// ADC r (Add Register with Carry)
		// (A) <- (A) + (r) + (CY)
		m_opcodes[0210] = [=]() { add(m_reg.B, GetFlag(FLAG_CY)); }; // B
		m_opcodes[0211] = [=]() { add(m_reg.C, GetFlag(FLAG_CY)); }; // C
		m_opcodes[0212] = [=]() { add(m_reg.D, GetFlag(FLAG_CY)); }; // D
		m_opcodes[0213] = [=]() { add(m_reg.E, GetFlag(FLAG_CY)); }; // E
		m_opcodes[0214] = [=]() { add(m_reg.H, GetFlag(FLAG_CY)); }; // H
		m_opcodes[0215] = [=]() { add(m_reg.L, GetFlag(FLAG_CY)); }; // L
		m_opcodes[0217] = [=]() { add(m_reg.A, GetFlag(FLAG_CY)); }; // A

		// ADC M (Add Memory with Carry)
		// (A) <- (A) + ((H)(L)) + (CY)
		m_opcodes[0216] = [=]() { add(ReadMem(), GetFlag(FLAG_CY)); }; // m

		// ACI data (Add immediate with Carry)
		// (A) <- (A) + (byte2) + (CY)
		m_opcodes[0316] = [=]() { add(FetchByte(), GetFlag(FLAG_CY)); };

		// SUB r (Substract Register)
		// (A) <- (A) - (r)
		m_opcodes[0220] = [=]() { sub(m_reg.B); }; // B
		m_opcodes[0221] = [=]() { sub(m_reg.C); }; // C
		m_opcodes[0222] = [=]() { sub(m_reg.D); }; // D
		m_opcodes[0223] = [=]() { sub(m_reg.E); }; // E
		m_opcodes[0224] = [=]() { sub(m_reg.H); }; // H
		m_opcodes[0225] = [=]() { sub(m_reg.L); }; // L
		m_opcodes[0227] = [=]() { sub(m_reg.A); }; // A

		// SUB M (Substract memory)
		// (A) <- (A) - ((H)(L))
		m_opcodes[0226] = [=]() { sub(ReadMem()); };	// m

		// SUI data (Substract immediate)
		// (A) <- (A) - (byte2)
		m_opcodes[0326] = [=]() { sub(FetchByte()); };

		// SBB r (Substract Register with Borrow)
		// (A) <- (A) - (r) - (CY)
		m_opcodes[0230] = [=]() { sub(m_reg.B, GetFlag(FLAG_CY)); }; // B
		m_opcodes[0231] = [=]() { sub(m_reg.C, GetFlag(FLAG_CY)); }; // C
		m_opcodes[0232] = [=]() { sub(m_reg.D, GetFlag(FLAG_CY)); }; // D
		m_opcodes[0233] = [=]() { sub(m_reg.E, GetFlag(FLAG_CY)); }; // E
		m_opcodes[0234] = [=]() { sub(m_reg.H, GetFlag(FLAG_CY)); }; // H
		m_opcodes[0235] = [=]() { sub(m_reg.L, GetFlag(FLAG_CY)); }; // L
		m_opcodes[0237] = [=]() { sub(m_reg.A, GetFlag(FLAG_CY)); }; // A

		// SBB M (Substract Memory with Borrow)
		// (A) <- (A) - ((H)(L)) - (CY)
		m_opcodes[0236] = [=]() { sub(ReadMem(), GetFlag(FLAG_CY)); }; // m

		// SBI data (Substract immediate with Borrow)
		// (A) <- (A) - (byte2) - (CY)
		m_opcodes[0336] = [=]() { sub(FetchByte(), GetFlag(FLAG_CY)); };

		// INR r (Increment Register)
		// (r) <- (r) + 1
		// Note: CY not affected
		m_opcodes[0004] = [=]() { inc(m_reg.B); }; // B
		m_opcodes[0014] = [=]() { inc(m_reg.C); }; // C
		m_opcodes[0024] = [=]() { inc(m_reg.D); }; // D
		m_opcodes[0034] = [=]() { inc(m_reg.E); }; // E
		m_opcodes[0044] = [=]() { inc(m_reg.H); }; // H
		m_opcodes[0054] = [=]() { inc(m_reg.L); }; // L
		m_opcodes[0074] = [=]() { inc(m_reg.A); }; // A

		// INR M (Increment Memory)
		// ((H)(L)) <- ((H)(L)) + 1
		// Note: CY not affected
		m_opcodes[0064] = [=]() { INRm(); }; // m

		// DCR r (Decrement Register)
		// (r) <- (r) - 1
		// Note: CY not affected
		m_opcodes[0005] = [=]() { dec(m_reg.B); }; // B
		m_opcodes[0015] = [=]() { dec(m_reg.C); }; // C
		m_opcodes[0025] = [=]() { dec(m_reg.D); }; // D
		m_opcodes[0035] = [=]() { dec(m_reg.E); }; // E
		m_opcodes[0045] = [=]() { dec(m_reg.H); }; // H
		m_opcodes[0055] = [=]() { dec(m_reg.L); }; // L
		m_opcodes[0075] = [=]() { dec(m_reg.A); }; // A
	
		// DCR M (Decrement Memory)
		// ((H)(L)) <- ((H)(L)) - 1
		// Note: CY not affected
		m_opcodes[0065] = [=]() { DCRm(); }; // m

		// INX rp (Increment Register Pair)
		// (rh)(rl) <- (rh)(rl) + 1
		// Note: No condition flags are affected
		m_opcodes[0003] = [=]() { INX(m_reg.B, m_reg.C); }; // BC
		m_opcodes[0023] = [=]() { INX(m_reg.D, m_reg.E); }; // DE
		m_opcodes[0043] = [=]() { INX(m_reg.H, m_reg.L); }; // HL
		m_opcodes[0063] = [=]() { ++m_regSP; };             // SP

		// DCX rp (Decrement Register Pair)
		// (rh)(rl) <- (rh)(rl) - 1
		// Note: No condition flags are affected
		m_opcodes[0013] = [=]() { DCX(m_reg.B, m_reg.C); }; // BC
		m_opcodes[0033] = [=]() { DCX(m_reg.D, m_reg.E); }; // DE
		m_opcodes[0053] = [=]() { DCX(m_reg.H, m_reg.L); }; // HL
		m_opcodes[0073] = [=]() { --m_regSP; };             // SP

		// DAD rp (Add register pair to H and L)
		// (H)(L) <- (H)(L) + (rh)(rl)
		// Note: Only the CY flag is affected
		m_opcodes[0011] = [=]() { dad(GetBC()); }; // BC
		m_opcodes[0031] = [=]() { dad(GetDE()); }; // DE
		m_opcodes[0051] = [=]() { dad(GetHL()); }; // HL
		m_opcodes[0071] = [=]() { dad(m_regSP); }; // SP

		// DAA (Decimal Adjust Accumulator)
		// The eight-bit number in the accumulator is adjusted to
		// form two four-bit Binary-Coded-Decimal digits by the 
		// following process:
		//
		// 1. If the value of the least significant 4 bits of the 
		//    accumulator is greater than 9 or if the AC flag is set,
		//    6 is added to the accumulator.
		// 2. If the value of the most significant 4 bits of the 
		//    accumulator is now greater than 9, *or* if the CY flag
		//    is set, 6 is added to the most significant 4 bits of
		//    the accumulator.
		// Note: All flags are adjusted
		m_opcodes[0047] = [=]() { DAA(); }; // TODO: Not implemented

		// -------------------
		// 3. Logical Group

		// Unless indicated otherwise, all instructions in this group
		// affect the Zero, Sign, Parity, Carry and Auxiliary Carry
		// flags according to the standard rules.

		// ANA r (AND Register)
		// (A) <- (A) and (r)
		// Note: The CY flag is cleared and the AC is set (8085)
		// The CY flag is cleared and AC is set to the ORing of 
		// bits 3 of the operands (8080)
		m_opcodes[0240] = [=]() { ana(m_reg.B); };	// B
		m_opcodes[0241] = [=]() { ana(m_reg.C); };	// C
		m_opcodes[0242] = [=]() { ana(m_reg.D); };	// D
		m_opcodes[0243] = [=]() { ana(m_reg.E); };	// E
		m_opcodes[0244] = [=]() { ana(m_reg.H); };	// H
		m_opcodes[0245] = [=]() { ana(m_reg.L); };	// L
		m_opcodes[0247] = [=]() { ana(m_reg.A); };	// A

		// ANA M (AND Memory)
		// (A) <- (A) and ((H)(L))
		// Note: same CY rules as ANA
		m_opcodes[0246] = [=]() { ana(ReadMem()); }; // m

		// ANI data (AND Immediate)
		// Note: same CY rules as ANA
		// (A) <- (A) and (byte 2)
		m_opcodes[0346] = [=]() { ana(FetchByte()); };

		// XRA r (XOR Register)
		// (A) <- (A) xor (r)
		// Note: The CY  and AC flags are cleared
		m_opcodes[0250] = [=]() { xra(m_reg.B); };	// B
		m_opcodes[0251] = [=]() { xra(m_reg.C); };	// C
		m_opcodes[0252] = [=]() { xra(m_reg.D); };	// D
		m_opcodes[0253] = [=]() { xra(m_reg.E); };	// E
		m_opcodes[0254] = [=]() { xra(m_reg.H); };	// H
		m_opcodes[0255] = [=]() { xra(m_reg.L); };	// L
		m_opcodes[0257] = [=]() { xra(m_reg.A); };	// A

		// XRA M (XOR Memory)
		// (A) <- (A) xor ((H)(L))
		// Note: The CY  and AC flags are cleared
		m_opcodes[0256] = [=]() { xra(ReadMem()); }; // m

		// XRI data (XOR Immediate)
		// (A) <- (A) xor (byte 2)
		// Note: The CY  and AC flags are cleared
		m_opcodes[0356] = [=]() { xra(FetchByte()); };

		// ORA r (OR Register)
		// (A) <- (A) or (r)
		// Note: The CY  and AC flags are cleared
		m_opcodes[0260] = [=]() { ora(m_reg.B); };	// B
		m_opcodes[0261] = [=]() { ora(m_reg.C); };	// C
		m_opcodes[0262] = [=]() { ora(m_reg.D); };	// D
		m_opcodes[0263] = [=]() { ora(m_reg.E); };	// E
		m_opcodes[0264] = [=]() { ora(m_reg.H); };	// H
		m_opcodes[0265] = [=]() { ora(m_reg.L); };	// L
		m_opcodes[0267] = [=]() { ora(m_reg.A); };	// A

		// ORA M (OR Memory)
		// (A) <- (A) or ((H)(L))
		// Note: The CY  and AC flags are cleared
		m_opcodes[0266] = [=]() { ora(ReadMem()); }; // m

		// ORI data (OR Immediate)
		// (A) <- (A) oor (byte 2)
		// Note: The CY  and AC flags are cleared
		m_opcodes[0366] = [=]() { ora(FetchByte()); };

		// CMP r (Compare Register)
		// (A) - (r)
		// Note: The accumulator remains unchanged.
		// The condition flags are set are set as
		// the result of the substraction.  The Z flag
		// is set to 1 if (A) = (r).  The CY flag is set
		// to 1 if (A) < (r)
		m_opcodes[0270] = [=]() { cmp(m_reg.B); };	// B
		m_opcodes[0271] = [=]() { cmp(m_reg.C); };	// C
		m_opcodes[0272] = [=]() { cmp(m_reg.D); };	// D
		m_opcodes[0273] = [=]() { cmp(m_reg.E); };	// E
		m_opcodes[0274] = [=]() { cmp(m_reg.H); };	// H
		m_opcodes[0275] = [=]() { cmp(m_reg.L); };	// L
		m_opcodes[0277] = [=]() { cmp(m_reg.A); };	// A

		// CMP M (Compare Memory)
		// (A) - ((H)(L))
		// Note: Same rules as CMP r
		m_opcodes[0276] = [=]() { cmp(ReadMem()); }; // m

		// CPI data (Compare immediate)
		// (A) - (byte2)
		// Note Same rules as CMP r
		m_opcodes[0376] = [=]() { cmp(FetchByte()); };

		// RLC (Rotate left)
		// (An+1) <- (An)
		// (A0) <- (A7)
		// (CY) <- (A7)
		// Note: Only the CY flag is affected
		m_opcodes[0007] = [=]() { RLC(); };

		// RRC (Rotate right)
		// (An) <- (An+1)
		// (A7) <- (A0)
		// (CY) <- (A0)
		// Note: Only the CY flag is affected
		m_opcodes[0017] = [=]() { RRC(); };

		// RAL (Rotate left through carry)
		// (An+1) <- (An)
		// (CY) <- (A7)
		// (A0) <- (CY)
		// Note: Only the CY flag is affected
		m_opcodes[0027] = [=]() { RAL(); };

		// RAL (Rotate right through carry)
		// (An) <- (An+1)
		// (CY) <- (A0)
		// (A7) <- (CY)
		// Note: Only the CY flag is affected
		m_opcodes[0037] = [=]() { RAR(); };

		// CMA (Complement accumulator)
		// (A) <- /(A)
		// Note: No flags are affected
		m_opcodes[0057] = [=]() { m_reg.A = ~m_reg.A; };

		// CMC (Complement carry)
		// (CY) <- /(CY)
		// Note: No other flags are affected
		m_opcodes[0077] = [=]() { ComplementFlag(FLAG_CY); };
	
		// STC (Set Carry)
		// (CY) <- 1
		// Note: No other flags are affected
		m_opcodes[0067] = [=]() { SetFlag(FLAG_CY, true); };

		// -------------------
		// 4. Branch Group

		// Condition flags are not affected by any instruction in this group
		//
		// CONDITION                 CCC
		// NZ - Not Zero    ( Z = 0) 000
		//  Z - Zero        ( Z = 1) 001
		// NC - No Carry    (CY = 0) 010
		//  C - Carry       (CY = 1) 011
		// PO - Parity odd  ( P = 0) 100
		// PE - Parity even ( P = 1) 101
		//  P - Plus        ( S = 0) 110
		//  M - Minus       ( S = 1) 111

		// JMP addr (Jump)
		// (PC) <- (byte3)(byte2)
		m_opcodes[0303] = [=]() { jumpIF(true); };
		m_opcodes[0313] = [=]() { jumpIF(true); }; // Undocumented

		// Jcondition addr (Conditional Jump)
		// If (CCC),
		// (PC) <- (byte3)(byte2)
		m_opcodes[0302] = [=]() { jumpIF(GetFlag(FLAG_Z) == false); };  // JNZ
		m_opcodes[0312] = [=]() { jumpIF(GetFlag(FLAG_Z) == true); };   // JZ
		m_opcodes[0322] = [=]() { jumpIF(GetFlag(FLAG_CY) == false); }; // JNC
		m_opcodes[0332] = [=]() { jumpIF(GetFlag(FLAG_CY) == true); };  // JC
		m_opcodes[0342] = [=]() { jumpIF(GetFlag(FLAG_P) == false); };  // JPO
		m_opcodes[0352] = [=]() { jumpIF(GetFlag(FLAG_P) == true); };   // JPE
		m_opcodes[0362] = [=]() { jumpIF(GetFlag(FLAG_S) == false); };  // JP
		m_opcodes[0372] = [=]() { jumpIF(GetFlag(FLAG_S) == true); };   // JM

		// CALL addr (Call)
		// ((SP) - 1) <- (PCH)
		// ((SP) - 2) <- (PCL)
		// (SP) <- (SP) - 2
		// (PC) <- (byte3)(byte2)
		m_opcodes[0315] = [=]() { callIF(true); };
		m_opcodes[0335] = [=]() { callIF(true); }; // Undocumented
		m_opcodes[0355] = [=]() { callIF(true); }; // Undocumented
		m_opcodes[0375] = [=]() { callIF(true); }; // Undocumented

		// Ccondition addr (Conditional Call)
		// If (CCC),
		// ((SP) - 1) <- (PCH)
		// ((SP) - 2) <- (PCL)
		// (SP) <- (SP) - 2
		// (PC) <- (byte3)(byte2)
		m_opcodes[0304] = [=]() { callIF(GetFlag(FLAG_Z) == false); };  // CNZ
		m_opcodes[0314] = [=]() { callIF(GetFlag(FLAG_Z) == true); };   // CZ
		m_opcodes[0324] = [=]() { callIF(GetFlag(FLAG_CY) == false); }; // CNC
		m_opcodes[0334] = [=]() { callIF(GetFlag(FLAG_CY) == true); };  // CC
		m_opcodes[0344] = [=]() { callIF(GetFlag(FLAG_P) == false); };  // CPO
		m_opcodes[0354] = [=]() { callIF(GetFlag(FLAG_P) == true); };   // CPE
		m_opcodes[0364] = [=]() { callIF(GetFlag(FLAG_S) == false); };  // CP
		m_opcodes[0374] = [=]() { callIF(GetFlag(FLAG_S) == true); };   // CM

		// RET (Return)
		// (PCL) <- ((SP))
		// (PCH) <- ((SP) + 1)
		// (SP) <- (SP) + 2
		m_opcodes[0311] = [=]() { retIF(true); };
		m_opcodes[0331] = [=]() { retIF(true); }; // Undocumented

		// Rcondition (Conditional Return)
		// If (CCC),
		// (PCL) <- ((SP))
		// (PCH) <- ((SP) + 1)
		// (SP) <- (SP) + 2
		m_opcodes[0300] = [=]() { retIF(GetFlag(FLAG_Z) == false); };  // RNZ
		m_opcodes[0310] = [=]() { retIF(GetFlag(FLAG_Z) == true); };   // RZ
		m_opcodes[0320] = [=]() { retIF(GetFlag(FLAG_CY) == false); }; // RNC
		m_opcodes[0330] = [=]() { retIF(GetFlag(FLAG_CY) == true); };  // RC
		m_opcodes[0340] = [=]() { retIF(GetFlag(FLAG_P) == false); };  // RPO
		m_opcodes[0350] = [=]() { retIF(GetFlag(FLAG_P) == true); };   // RPE
		m_opcodes[0360] = [=]() { retIF(GetFlag(FLAG_S) == false); };  // RP
		m_opcodes[0370] = [=]() { retIF(GetFlag(FLAG_S) == true); };   // RM

		// RST n (Restart)
		// ((SP) - 1) <- (PCH)
		// ((SP) - 2) <- (PCL)
		// (SP) <- (SP) - 2
		// (PC) <- 8 * NNN
		m_opcodes[0307] = [=]() { RST(0); };
		m_opcodes[0317] = [=]() { RST(1); };
		m_opcodes[0327] = [=]() { RST(2); };
		m_opcodes[0337] = [=]() { RST(3); };
		m_opcodes[0347] = [=]() { RST(4); };
		m_opcodes[0357] = [=]() { RST(5); };
		m_opcodes[0367] = [=]() { RST(6); };
		m_opcodes[0377] = [=]() { RST(7); };

		// PCHL (Jump H and L indirect - move H and L to PC)
		// (PCH)<-(H)
		// (PCL)<-(L)
		m_opcodes[0351] = [=]() { m_programCounter = GetHL(); };

		// -------------------
		// 5. Stack, I/O and Machine Control Group

		// PUSH rp (Push)
		// ((SP) - 1)<-(rh)
		// ((SP) - 2)<-(rl)
		// (SP)<-(SP)-2
		// Note: Register pair rp=SP may not be specified
		m_opcodes[0305] = [=]() { push(m_reg.B, m_reg.C); }; // BC
		m_opcodes[0325] = [=]() { push(m_reg.D, m_reg.E); }; // DE
		m_opcodes[0345] = [=]() { push(m_reg.H, m_reg.L); }; // HL

		// PUSH PSW (Push processor status word)
		// ((SP) - 1)<-(A)
		// ((SP) - 2)<-(FLAGS)
		// (SP)<-(SP)-2
		m_opcodes[0365] = [=]() { push(m_reg.A, m_reg.flags); };

		// POP rp (Pop)
		// (rl) <- ((SP))
		// (rh) <- ((SP) + 1)
		// (SP) <- (SP)+2
		// Note: Register pair rp=SP may not be specified
		m_opcodes[0301] = [=]() { pop(m_reg.B, m_reg.C); }; // BC
		m_opcodes[0321] = [=]() { pop(m_reg.D, m_reg.E); }; // DE
		m_opcodes[0341] = [=]() { pop(m_reg.H, m_reg.L); }; // HL

		// POP PSW (Pop processor status word)
		// (FLAGS)<- ((SP))
		// (A)    <- ((SP)+1)
		// (SP)   <- (SP)+2
		m_opcodes[0361] = [=]() { BYTE f; pop(m_reg.A, f); SetFlags(f); };

		//XTHL (Exchange stack top with H and L)
		// (L) <- ((SP))
		// (H) <- ((SP) + 1)
		m_opcodes[0343] = [=]() { XTHL(); };

		// SPHL (Move HL to SP)
		// (SP) <- (H)(L)
		m_opcodes[0371] = [=]() { SPHL(); };

		// IN port (Input)
		// (A) <- (data)
		m_opcodes[0333] = [=]() { In(FetchByte(), m_reg.A); };

		// OUT port (Output)
		// (data) <- (A)
		m_opcodes[0323] = [=]() { Out(FetchByte(), m_reg.A); };

		// EI (Enable Interrupts)
		// The interrupt system is enabled following the execution
		// of the next instruction. 
		m_opcodes[0373] = [=]() { EI(); };

		// DI (Disable Interrupts)
		// The interrupt ststem is disabled immediately following
		// the execution of the DI instruction.  Interrupts are
		// not recognized during the DI instruction
		m_opcodes[0363] = [=]() { DI(); };

		// HLT (Halt)
		m_opcodes[0166] = [=]() { --m_programCounter; Halt(); };

		// NOP (No op)
		m_opcodes[0000] = [=]() { };

		// Illegal/Undocumented (NOP)
		m_opcodes[0010] = [=]() { };
		m_opcodes[0020] = [=]() { };
		m_opcodes[0030] = [=]() { };
		m_opcodes[0040] = [=]() { };
		m_opcodes[0050] = [=]() { };
		m_opcodes[0060] = [=]() { };
		m_opcodes[0070] = [=]() { };
	}

	CPU8080::~CPU8080()
	{
	}

	void CPU8080::Reset()
	{
		CPU::Reset();

		m_reg.A = 0;
		m_reg.B = 0;
		m_reg.C = 0;
		m_reg.D = 0;
		m_reg.E = 0;
		m_reg.H = 0;
		m_reg.L = 0;

		m_regSP = 0;
		m_programCounter = 0;

		ClearFlags(m_reg.flags);
	}

	void CPU8080::ClearFlags(BYTE& flags)
	{	
		flags = FLAG_RESERVED_ON;
	}

	void CPU8080::SetFlags(BYTE f)
	{
		SetBitMask(f, FLAG_RESERVED_OFF, false);
		SetBitMask(f, FLAG_RESERVED_ON, true);
		m_reg.flags = f;
	}

	BYTE CPU8080::FetchByte()
	{
		BYTE b = m_memory.Read8(GetCurrentAddress());
		++m_programCounter;
		return b;
	}

	WORD CPU8080::FetchWord()
	{
		BYTE l = m_memory.Read8(GetCurrentAddress());
		++m_programCounter;
		BYTE h = m_memory.Read8(GetCurrentAddress());
		++m_programCounter;

		return MakeWord(h, l);
	}

	bool CPU8080::Step()
	{
		bool ret = true;
		if (m_state != CPUState::HALT)
		{
			ret = CPU::Step();
		}

		if (m_state == CPUState::HALT)
		{
			Halt();
			m_opTicks = 1;
			ret = true;
		}
		
		if (ret)
		{
			Interrupt();
		}

		return ret;
	}

	void CPU8080::Dump()
	{
		LogPrintf(LOG_DEBUG, "AF = %02X %02X\tCY = %c", m_reg.A, m_reg.flags, GetFlag(FLAG_CY)?'1':'0');
		LogPrintf(LOG_DEBUG, "BC = %02X %02X\tP  = %c", m_reg.B, m_reg.C, GetFlag(FLAG_P)?'1':'0');
		LogPrintf(LOG_DEBUG, "DE = %02X %02X\tAC = %c", m_reg.D, m_reg.E, GetFlag(FLAG_AC)?'1':'0');
		LogPrintf(LOG_DEBUG, "HL = %02X %02X\tZ  = %c", m_reg.H, m_reg.L, GetFlag(FLAG_Z)?'1':'0');
		LogPrintf(LOG_DEBUG, "SP = %04X   \tS  = %c", m_regSP, GetFlag(FLAG_S)?'1':'0');
		LogPrintf(LOG_DEBUG, "PC = %04X", m_programCounter);
		LogPrintf(LOG_DEBUG, "");
	}

	void CPU8080::UnknownOpcode()
	{
		LogPrintf(LOG_ERROR, "CPU: Unknown Opcode (0x%02X) at address 0x%04X", m_opcode, m_programCounter);
	}

	void CPU8080::Exec(BYTE opcode)
	{
		++m_programCounter;

		m_opcode = opcode;
		m_currTiming = &m_info.GetOpcodeTiming(opcode);

		try
		{
			// Fetch the function corresponding to the opcode and run it
			{
				auto& opFunc = m_opcodes[opcode];
				opFunc();
			}

			TICK();
		}
		catch (std::exception e)
		{
			LogPrintf(LOG_ERROR, "CPU: Exception at address 0x%04X! Stopping CPU", m_programCounter);
			m_state = CPUState::STOP;
		}
	}

	void CPU8080::Interrupt()
	{
		if (m_interruptsEnabled)
		{
			return;
		}

		// TODO: Not implemented
	}

	void CPU8080::AdjustBaseFlags(BYTE val)
	{
		SetFlag(FLAG_Z, (val == 0));
		SetFlag(FLAG_S, GetBit(val, 7));
		SetFlag(FLAG_P, IsParityEven(val));
	}

	void CPU8080::LXIb()
	{
		m_reg.C = FetchByte();
		m_reg.B = FetchByte();
	}

	void CPU8080::LXId()
	{
		m_reg.E = FetchByte();
		m_reg.D = FetchByte();
	}

	void CPU8080::LXIh()
	{
		m_reg.L = FetchByte();
		m_reg.H = FetchByte();
	}

	void CPU8080::STAXb()
	{
		m_memory.Write8(GetBC(), m_reg.A);
	}

	void CPU8080::STAXd()
	{
		m_memory.Write8(GetDE(), m_reg.A);
	}

	void CPU8080::LDAXb()
	{
		m_reg.A = m_memory.Read8(GetBC());
	}

	void CPU8080::LDAXd()
	{
		m_reg.A = m_memory.Read8(GetDE());
	}

	void CPU8080::STA()
	{
		ADDRESS dest = FetchWord();
		m_memory.Write8(dest, m_reg.A);
	}

	void CPU8080::LDA()
	{
		ADDRESS src = FetchWord();
		m_reg.A = m_memory.Read8(src);
	}

	void CPU8080::SHLD()
	{
		ADDRESS dest = FetchWord();
		m_memory.Write16(dest, GetHL());
	}

	void CPU8080::LHLD()
	{
		ADDRESS src = FetchWord();
		m_reg.L = m_memory.Read8(src);
		m_reg.H = m_memory.Read8(src + 1);
	}

	void CPU8080::XCHG()
	{
		BYTE oldH, oldL;

		oldL = m_reg.L; oldH = m_reg.H;
	
		m_reg.L = m_reg.E; m_reg.H = m_reg.D;
		m_reg.E = oldL; m_reg.D = oldH;
	}

	void CPU8080::push(BYTE h, BYTE l)
	{
		m_memory.Write8(--m_regSP, h);
		m_memory.Write8(--m_regSP, l);
	}

	void CPU8080::pushPC()
	{
		push(GetHByte(m_programCounter), GetLByte(m_programCounter));
	}

	void CPU8080::pop(BYTE &h, BYTE &l)
	{
		l = m_memory.Read8(m_regSP++);
		h = m_memory.Read8(m_regSP++);
	}

	void CPU8080::popPC()
	{
		BYTE h, l;
		pop(h, l);
		m_programCounter = MakeWord(h, l);
	}

	void CPU8080::XTHL()
	{
		BYTE oldH, oldL;

		oldL = m_reg.L; oldH = m_reg.H;
	
		m_reg.L = m_memory.Read8(m_regSP);
		m_reg.H = m_memory.Read8(m_regSP+1);
	
		m_memory.Write8(m_regSP, oldL);
		m_memory.Write8(m_regSP+1, oldH);
	}

	void CPU8080::SPHL()
	{
		m_regSP = GetHL();
	}

	void CPU8080::LXIsp()
	{
		m_regSP = FetchWord();
	}

	void CPU8080::jumpIF(bool condition)
	{
		ADDRESS dest = FetchWord();
		if (condition == true)
		{
			m_programCounter = dest;
			TICKT3();
		}
	}

	void CPU8080::callIF(bool condition)
	{
		ADDRESS dest = FetchWord();

		if (condition == true)
		{
			pushPC();
			m_programCounter = dest;
			TICKT3();
		}
	}

	void CPU8080::retIF(bool condition)
	{
		if (condition == true)
		{
			popPC();

			TICKT3();
		}
	}

	void CPU8080::RST(BYTE rst)
	{
		pushPC();

		m_programCounter = (rst << 3);
	}

	void CPU8080::inc(BYTE& reg)
	{
		reg++;

		AdjustBaseFlags(reg);
		SetFlag(FLAG_AC, (reg & 0x0F) == 0);
	}

	void CPU8080::dec(BYTE& reg)
	{
		reg--;

		AdjustBaseFlags(reg);
		SetFlag(FLAG_AC, (reg & 0x0F) == 0x0F);
	}

	void CPU8080::INRm()
	{
		BYTE value = ReadMem();
		inc(value);
		WriteMem(value);
	}

	void CPU8080::DCRm()
	{
		BYTE value = ReadMem();
		dec(value);
		WriteMem(value);
	}

	void CPU8080::INX(BYTE& h, BYTE& l)
	{
		WORD value = MakeWord(h, l);
		value++;
		h = GetHByte(value);
		l = GetLByte(value);
	}

	void CPU8080::DCX(BYTE& h, BYTE& l)
	{
		WORD value = MakeWord(h, l);
		value--;
		h = GetHByte(value);
		l = GetLByte(value);
	}

	void CPU8080::add(BYTE src, bool carry)
	{
		// AC flag
		BYTE loNibble = (m_reg.A & 0x0F) + (src & 0x0F);

		WORD temp = m_reg.A + src;
		if (carry)
		{
			temp++;
			loNibble++;
		}

		m_reg.A = (BYTE)temp;

		AdjustBaseFlags(m_reg.A);
		SetFlag(FLAG_CY, (temp > 0xFF));
		SetFlag(FLAG_AC, (loNibble > 0x0F));
	}

	void CPU8080::dad(WORD value)
	{
		DWORD hl = GetHL();

		hl += value;

		SetFlag(FLAG_CY, hl > 0xFFFF);

		m_reg.H = GetHByte((WORD)hl);
		m_reg.L = GetLByte((WORD)hl);
	}

	void CPU8080::sub(BYTE src, bool borrow)
	{
		// AC flag
		char loNibble = (m_reg.A & 0x0F) - (src & 0x0F);

		int temp = m_reg.A - src;
		if (borrow)
		{
			temp--;
			loNibble--;
		}

		m_reg.A = (BYTE)temp;

		AdjustBaseFlags(m_reg.A);
		SetFlag(FLAG_CY, (temp < 0));
		SetFlag(FLAG_AC, !(loNibble < 0));
	}

	void CPU8080::ana(BYTE src)
	{
		m_reg.A &= src;

		AdjustBaseFlags(m_reg.A);
		SetFlag(FLAG_AC, true);
		SetFlag(FLAG_CY, false);
	}

	void CPU8080::xra(BYTE src)
	{
		m_reg.A ^= src;

		AdjustBaseFlags(m_reg.A);
		SetFlag(FLAG_AC, false);
		SetFlag(FLAG_CY, false);
	}

	void CPU8080::ora(BYTE src)
	{
		m_reg.A |= src;

		AdjustBaseFlags(m_reg.A);
		SetFlag(FLAG_AC, false);
		SetFlag(FLAG_CY, false);
	}

	BYTE CPU8080::cmp(BYTE src)
	{
		// AC flag
		char loNibble = (m_reg.A & 0x0F) - (src & 0x0F);

		int temp = m_reg.A - src;

		SetFlag(FLAG_CY, (temp < 0));
		SetFlag(FLAG_AC, !(loNibble < 0));
		AdjustBaseFlags(temp);
		return temp;
	}

	void CPU8080::RLC()
	{
		bool msb = (m_reg.A & 128);

		m_reg.A = (m_reg.A << 1);
		m_reg.A |= (msb ? 1 : 0);

		SetFlag(FLAG_CY, msb);
	}

	void CPU8080::RRC()
	{
		bool lsb = (m_reg.A & 1);

		m_reg.A = (m_reg.A >> 1);
		m_reg.A &= ~128;
		m_reg.A |= (lsb ? 128 : 0);

		SetFlag(FLAG_CY, lsb);
	}

	void CPU8080::RAL()
	{
		bool msb = (m_reg.A & 128);

		m_reg.A = (m_reg.A << 1);
		m_reg.A |= (GetFlag(FLAG_CY) ? 1 : 0);

		SetFlag(FLAG_CY, msb);
	}

	void CPU8080::RAR()
	{
		bool lsb = (m_reg.A & 1);

		m_reg.A = (m_reg.A >> 1);
		m_reg.A &= ~128;
		m_reg.A |= (GetFlag(FLAG_CY) ? 128 : 0);

		SetFlag(FLAG_CY, lsb);
	}

	void CPU8080::DAA()
	{
		// TODO
		LogPrintf(LOG_WARNING, "DAA: Not implemented");
	}

	void CPU8080::EI()
	{
		m_interruptsEnabled = true; // TODO: Should happen after next instruction execution
	}

	void CPU8080::DI()
	{
		m_interruptsEnabled = false;
	}

	void CPU8080::NOP()
	{
	}
}
