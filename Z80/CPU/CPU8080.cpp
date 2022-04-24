#include "stdafx.h"
#include "CPU8080.h"

using cpuInfo::Opcode;
using cpuInfo::CPUType;

namespace emul
{
	CPU8080::CPU8080(Memory& memory, Interrupts& interrupts) : CPU8080(CPUType::i8080, memory, interrupts)
	{
	}


	CPU8080::CPU8080(cpuInfo::CPUType type, Memory& memory, Interrupts& interrupts) :
		CPU(CPU8080_ADDRESS_BITS, memory),
		m_info(type),
		m_interrupts(interrupts),
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
		m_opcodes[0100] = [=]() { regB = regB; }; // B,B
		m_opcodes[0101] = [=]() { regB = regC; }; // B,C
		m_opcodes[0102] = [=]() { regB = regD; }; // B,D
		m_opcodes[0103] = [=]() { regB = regE; }; // B,E
		m_opcodes[0104] = [=]() { regB = regH; }; // B,H
		m_opcodes[0105] = [=]() { regB = regL; }; // B,L
		m_opcodes[0107] = [=]() { regB = regA; }; // B,A

		// Destination C
		m_opcodes[0110] = [=]() { regC = regB; }; // C,B
		m_opcodes[0111] = [=]() { regC = regC; }; // C,C
		m_opcodes[0112] = [=]() { regC = regD; }; // C,D
		m_opcodes[0113] = [=]() { regC = regE; }; // C,E
		m_opcodes[0114] = [=]() { regC = regH; }; // C,H
		m_opcodes[0115] = [=]() { regC = regL; }; // C,L
		m_opcodes[0117] = [=]() { regC = regA; }; // C,A

		// Destination D
		m_opcodes[0120] = [=]() { regD = regB; }; // D,B
		m_opcodes[0121] = [=]() { regD = regC; }; // D,C
		m_opcodes[0122] = [=]() { regD = regD; }; // D,D
		m_opcodes[0123] = [=]() { regD = regE; }; // D,E
		m_opcodes[0124] = [=]() { regD = regH; }; // D,H
		m_opcodes[0125] = [=]() { regD = regL; }; // D,L
		m_opcodes[0127] = [=]() { regD = regA; }; // D,A

		// Destination E	
		m_opcodes[0130] = [=]() { regE = regB; }; // E,B
		m_opcodes[0131] = [=]() { regE = regC; }; // E,C
		m_opcodes[0132] = [=]() { regE = regD; }; // E,D
		m_opcodes[0133] = [=]() { regE = regE; }; // E,E
		m_opcodes[0134] = [=]() { regE = regH; }; // E,H
		m_opcodes[0135] = [=]() { regE = regL; }; // E,L
		m_opcodes[0137] = [=]() { regE = regA; }; // E,A
	
		// Destination H
		m_opcodes[0140] = [=]() { regH = regB; }; // H,B
		m_opcodes[0141] = [=]() { regH = regC; }; // H,C
		m_opcodes[0142] = [=]() { regH = regD; }; // H,D
		m_opcodes[0143] = [=]() { regH = regE; }; // H,E
		m_opcodes[0144] = [=]() { regH = regH; }; // H,H
		m_opcodes[0145] = [=]() { regH = regL; }; // H,L
		m_opcodes[0147] = [=]() { regH = regA; }; // H,A

		// Destination L
		m_opcodes[0150] = [=]() { regL = regB; }; // L,B
		m_opcodes[0151] = [=]() { regL = regC; }; // L,C
		m_opcodes[0152] = [=]() { regL = regD; }; // L,D
		m_opcodes[0153] = [=]() { regL = regE; }; // L,E
		m_opcodes[0154] = [=]() { regL = regH; }; // L,H
		m_opcodes[0155] = [=]() { regL = regL; }; // L,L
		m_opcodes[0157] = [=]() { regL = regA; }; // L,A

		// Destination A
		m_opcodes[0170] = [=]() { regA = regB; }; // A,B
		m_opcodes[0171] = [=]() { regA = regC; }; // A,C
		m_opcodes[0172] = [=]() { regA = regD; }; // A,D
		m_opcodes[0173] = [=]() { regA = regE; }; // A,E
		m_opcodes[0174] = [=]() { regA = regH; }; // A,H
		m_opcodes[0175] = [=]() { regA = regL; }; // A,L
		m_opcodes[0177] = [=]() { regA = regA; }; // A,A

		// MOV r, M (Move from memory)
		// (r) <- ((H)(L))
		m_opcodes[0106] = [=]() { regB = ReadMem(); }; // B,m
		m_opcodes[0116] = [=]() { regC = ReadMem(); }; // C,m
		m_opcodes[0126] = [=]() { regD = ReadMem(); }; // D,m
		m_opcodes[0136] = [=]() { regE = ReadMem(); }; // E,m
		m_opcodes[0146] = [=]() { regH = ReadMem(); }; // H,m
		m_opcodes[0156] = [=]() { regL = ReadMem(); }; // L,m
		m_opcodes[0176] = [=]() { regA = ReadMem(); }; // A,m

		// MOV M, r (Move to memory)
		// ((H)(L) <- (r)
		m_opcodes[0160] = [=]() { WriteMem(regB); }; // m,B
		m_opcodes[0161] = [=]() { WriteMem(regC); }; // m,C
		m_opcodes[0162] = [=]() { WriteMem(regD); }; // m,D
		m_opcodes[0163] = [=]() { WriteMem(regE); }; // m,E
		m_opcodes[0164] = [=]() { WriteMem(regH); }; // m,H
		m_opcodes[0165] = [=]() { WriteMem(regL); }; // m,L
		m_opcodes[0167] = [=]() { WriteMem(regA); }; // m,A

		// MVI r, data (Move immediate)
		// (r) <- (byte 2)
		m_opcodes[0006] = [=]() { regB = FetchByte(); }; // B
		m_opcodes[0016] = [=]() { regC = FetchByte(); }; // C
		m_opcodes[0026] = [=]() { regD = FetchByte(); }; // D
		m_opcodes[0036] = [=]() { regE = FetchByte(); }; // E
		m_opcodes[0046] = [=]() { regH = FetchByte(); }; // H
		m_opcodes[0056] = [=]() { regL = FetchByte(); }; // L
		m_opcodes[0076] = [=]() { regA = FetchByte(); }; // A

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
		m_opcodes[0200] = [=]() { add(regB); };	// B
		m_opcodes[0201] = [=]() { add(regC); };	// C
		m_opcodes[0202] = [=]() { add(regD); };	// D
		m_opcodes[0203] = [=]() { add(regE); };	// E
		m_opcodes[0204] = [=]() { add(regH); };	// H
		m_opcodes[0205] = [=]() { add(regL); };	// L
		m_opcodes[0207] = [=]() { add(regA); };	// A

		// ADD M (Add memory)
		// (A) <- (A) + ((H)(L))
		m_opcodes[0206] = [=]() { add(ReadMem()); }; // m

		// ADI data (Add immediate)
		// (A) <- (A) + (byte2)
		m_opcodes[0306] = [=]() { add(FetchByte()); };

		// ADC r (Add Register with Carry)
		// (A) <- (A) + (r) + (CY)
		m_opcodes[0210] = [=]() { add(regB, GetFlag(FLAG_CY)); }; // B
		m_opcodes[0211] = [=]() { add(regC, GetFlag(FLAG_CY)); }; // C
		m_opcodes[0212] = [=]() { add(regD, GetFlag(FLAG_CY)); }; // D
		m_opcodes[0213] = [=]() { add(regE, GetFlag(FLAG_CY)); }; // E
		m_opcodes[0214] = [=]() { add(regH, GetFlag(FLAG_CY)); }; // H
		m_opcodes[0215] = [=]() { add(regL, GetFlag(FLAG_CY)); }; // L
		m_opcodes[0217] = [=]() { add(regA, GetFlag(FLAG_CY)); }; // A

		// ADC M (Add Memory with Carry)
		// (A) <- (A) + ((H)(L)) + (CY)
		m_opcodes[0216] = [=]() { add(ReadMem(), GetFlag(FLAG_CY)); }; // m

		// ACI data (Add immediate with Carry)
		// (A) <- (A) + (byte2) + (CY)
		m_opcodes[0316] = [=]() { add(FetchByte(), GetFlag(FLAG_CY)); };

		// SUB r (Substract Register)
		// (A) <- (A) - (r)
		m_opcodes[0220] = [=]() { sub(regB); };	// B
		m_opcodes[0221] = [=]() { sub(regC); };	// C
		m_opcodes[0222] = [=]() { sub(regD); };	// D
		m_opcodes[0223] = [=]() { sub(regE); };	// E
		m_opcodes[0224] = [=]() { sub(regH); };	// H
		m_opcodes[0225] = [=]() { sub(regL); };	// L
		m_opcodes[0227] = [=]() { sub(regA); };	// A

		// SUB M (Substract memory)
		// (A) <- (A) - ((H)(L))
		m_opcodes[0226] = [=]() { sub(ReadMem()); };	// m

		// SUI data (Substract immediate)
		// (A) <- (A) - (byte2)
		m_opcodes[0326] = [=]() { sub(FetchByte()); };

		// SBB r (Substract Register with Borrow)
		// (A) <- (A) - (r) - (CY)
		m_opcodes[0230] = [=]() { sub(regB, GetFlag(FLAG_CY)); }; // B
		m_opcodes[0231] = [=]() { sub(regC, GetFlag(FLAG_CY)); }; // C
		m_opcodes[0232] = [=]() { sub(regD, GetFlag(FLAG_CY)); }; // D
		m_opcodes[0233] = [=]() { sub(regE, GetFlag(FLAG_CY)); }; // E
		m_opcodes[0234] = [=]() { sub(regH, GetFlag(FLAG_CY)); }; // H
		m_opcodes[0235] = [=]() { sub(regL, GetFlag(FLAG_CY)); }; // L
		m_opcodes[0237] = [=]() { sub(regA, GetFlag(FLAG_CY)); }; // A

		// SBB M (Substract Memory with Borrow)
		// (A) <- (A) - ((H)(L)) - (CY)
		m_opcodes[0236] = [=]() { sub(ReadMem(), GetFlag(FLAG_CY)); }; // m

		// SBI data (Substract immediate with Borrow)
		// (A) <- (A) - (byte2) - (CY)
		m_opcodes[0336] = [=]() { sub(FetchByte(), GetFlag(FLAG_CY)); };

		// INR r (Increment Register)
		// (r) <- (r) + 1
		// Note: CY not affected
		m_opcodes[0004] = [=]() { INRr(regB); }; // B
		m_opcodes[0014] = [=]() { INRr(regC); }; // C
		m_opcodes[0024] = [=]() { INRr(regD); }; // D
		m_opcodes[0034] = [=]() { INRr(regE); }; // E
		m_opcodes[0044] = [=]() { INRr(regH); }; // H
		m_opcodes[0054] = [=]() { INRr(regL); }; // L
		m_opcodes[0074] = [=]() { INRr(regA); }; // A

		// INR M (Increment Memory)
		// ((H)(L)) <- ((H)(L)) + 1
		// Note: CY not affected
		m_opcodes[0064] = [=]() { INRm(); }; // m

		// DCR r (Decrement Register)
		// (r) <- (r) - 1
		// Note: CY not affected
		m_opcodes[0005] = [=]() { DCRr(regB); }; // B
		m_opcodes[0015] = [=]() { DCRr(regC); }; // C
		m_opcodes[0025] = [=]() { DCRr(regD); }; // D
		m_opcodes[0035] = [=]() { DCRr(regE); }; // E
		m_opcodes[0045] = [=]() { DCRr(regH); }; // H
		m_opcodes[0055] = [=]() { DCRr(regL); }; // L
		m_opcodes[0075] = [=]() { DCRr(regA); }; // A
	
		// DCR M (Decrement Memory)
		// ((H)(L)) <- ((H)(L)) - 1
		// Note: CY not affected
		m_opcodes[0065] = [=]() { DCRm(); }; // m

		// INX rp (Increment Register Pair)
		// (rh)(rl) <- (rh)(rl) + 1
		// Note: No condition flags are affected
		m_opcodes[0003] = [=]() { INX(regB, regC); }; // BC
		m_opcodes[0023] = [=]() { INX(regD, regE); }; // DE
		m_opcodes[0043] = [=]() { INX(regH, regL); }; // HL
		m_opcodes[0063] = [=]() { ++regSP; };         // SP

		// DCX rp (Decrement Register Pair)
		// (rh)(rl) <- (rh)(rl) - 1
		// Note: No condition flags are affected
		m_opcodes[0013] = [=]() { DCX(regB, regC); }; // BC
		m_opcodes[0033] = [=]() { DCX(regD, regE); }; // DE
		m_opcodes[0053] = [=]() { DCX(regH, regL); }; // HL
		m_opcodes[0073] = [=]() { --regSP; };         // SP

		// DAD rp (Add register pair to H and L)
		// (H)(L) <- (H)(L) + (rh)(rl)
		// Note: Only the CY flag is affected
		m_opcodes[0011] = [=]() { dad(MakeWord(regB, regC)); }; // BC
		m_opcodes[0031] = [=]() { dad(MakeWord(regD, regE)); }; // DE
		m_opcodes[0051] = [=]() { dad(GetHL()); };              // HL
		m_opcodes[0071] = [=]() { dad(regSP); };                // SP

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
		m_opcodes[0240] = [=]() { ana(regB); };	// B
		m_opcodes[0241] = [=]() { ana(regC); };	// C
		m_opcodes[0242] = [=]() { ana(regD); };	// D
		m_opcodes[0243] = [=]() { ana(regE); };	// E
		m_opcodes[0244] = [=]() { ana(regH); };	// H
		m_opcodes[0245] = [=]() { ana(regL); };	// L
		m_opcodes[0247] = [=]() { ana(regA); };	// A

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
		m_opcodes[0250] = [=]() { xra(regB); };	// B
		m_opcodes[0251] = [=]() { xra(regC); };	// C
		m_opcodes[0252] = [=]() { xra(regD); };	// D
		m_opcodes[0253] = [=]() { xra(regE); };	// E
		m_opcodes[0254] = [=]() { xra(regH); };	// H
		m_opcodes[0255] = [=]() { xra(regL); };	// L
		m_opcodes[0257] = [=]() { xra(regA); };	// A

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
		m_opcodes[0260] = [=]() { ora(regB); };	// B
		m_opcodes[0261] = [=]() { ora(regC); };	// C
		m_opcodes[0262] = [=]() { ora(regD); };	// D
		m_opcodes[0263] = [=]() { ora(regE); };	// E
		m_opcodes[0264] = [=]() { ora(regH); };	// H
		m_opcodes[0265] = [=]() { ora(regL); };	// L
		m_opcodes[0267] = [=]() { ora(regA); };	// A

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
		m_opcodes[0270] = [=]() { cmp(regB); };	// B
		m_opcodes[0271] = [=]() { cmp(regC); };	// C
		m_opcodes[0272] = [=]() { cmp(regD); };	// D
		m_opcodes[0273] = [=]() { cmp(regE); };	// E
		m_opcodes[0274] = [=]() { cmp(regH); };	// H
		m_opcodes[0275] = [=]() { cmp(regL); };	// L
		m_opcodes[0277] = [=]() { cmp(regA); };	// A

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
		m_opcodes[0057] = [=]() { regA = ~regA; };

		// CMC (Complement carry)
		// (CY) <- /(CY)
		// Note: No other flags are affected
		m_opcodes[0077] = [=]() { SetFlag(FLAG_CY, !GetFlag(FLAG_CY)); };
	
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
		m_opcodes[0305] = [=]() { push(regB, regC); }; // BC
		m_opcodes[0325] = [=]() { push(regD, regE); }; // DE
		m_opcodes[0345] = [=]() { push(regH, regL); }; // HL

		// PUSH PSW (Push processor status word)
		// ((SP) - 1)<-(A)
		// ((SP) - 2)<-(FLAGS)
		// (SP)<-(SP)-2
		m_opcodes[0365] = [=]() { push(regA, flags); };

		// POP rp (Pop)
		// (rl) <- ((SP))
		// (rh) <- ((SP) + 1)
		// (SP) <- (SP)+2
		// Note: Register pair rp=SP may not be specified
		m_opcodes[0301] = [=]() { pop(regB, regC); }; // BC
		m_opcodes[0321] = [=]() { pop(regD, regE); }; // DE
		m_opcodes[0341] = [=]() { pop(regH, regL); }; // HL

		// POP PSW (Pop processor status word)
		// (FLAGS)<- ((SP))
		// (A)    <- ((SP)+1)
		// (SP)   <- (SP)+2
		m_opcodes[0361] = [=]() { BYTE f; pop(regA, f); SetFlags(f); };

		//XTHL (Exchange stack top with H and L)
		// (L) <- ((SP))
		// (H) <- ((SP) + 1)
		m_opcodes[0343] = [=]() { XTHL(); };

		// SPHL (Move HL to SP)
		// (SP) <- (H)(L)
		m_opcodes[0371] = [=]() { SPHL(); };

		// IN port (Input)
		// (A) <- (data)
		m_opcodes[0333] = [=]() { In(FetchByte(), regA); };

		// OUT port (Output)
		// (data) <- (A)
		m_opcodes[0323] = [=]() { Out(FetchByte(), regA); };

		// EI (Enable Interrupts)
		// The interrupt system is enabled following the execution
		// of the next instruction. 
		m_opcodes[0373] = [=]() { EI(); };

		// DI (Disable Interrupts)
		// The interrupt ststem is disabled immediately following
		// the execution of the DI instruction.  Interrupts are
		// not recognized during the DI instruction
		m_opcodes[0363] = [=]() { m_interruptsEnabled = false; };

		// HLT (Halt)
		m_opcodes[0166] = [=]() { HLT(); };

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

		regA = 0;
		regB = 0;
		regC = 0;
		regD = 0;
		regE = 0;
		regH = 0;
		regL = 0;

		regSP = 0;
		m_programCounter = 0;

		ClearFlags();
	}

	void CPU8080::ClearFlags()
	{
		flags = FLAG_RESERVED_ON;
	}

	void CPU8080::SetFlags(BYTE f)
	{
		SetBitMask(f, FLAG_RESERVED_OFF, false);
		SetBitMask(f, FLAG_RESERVED_ON, true);
		flags = f;
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
		if (!CPU::Step())
			return false;

		// Check for interrupts
		// TODO: No Timing, no latching, no masking... 
		if (m_interruptsEnabled)
		{
			interrupt(TRAP);
			interrupt(RST75);
			interrupt(RST65);
			interrupt(RST55);
		}

		return true;
	}

	void CPU8080::Dump()
	{
		LogPrintf(LOG_DEBUG, "AF = %02X %02X\tCY = %c", regA, flags, GetFlag(FLAG_CY)?'1':'0');
		LogPrintf(LOG_DEBUG, "BC = %02X %02X\tP  = %c", regB, regC, GetFlag(FLAG_P)?'1':'0');
		LogPrintf(LOG_DEBUG, "DE = %02X %02X\tAC = %c", regD, regE, GetFlag(FLAG_AC)?'1':'0');
		LogPrintf(LOG_DEBUG, "HL = %02X %02X\tZ  = %c", regH, regL, GetFlag(FLAG_Z)?'1':'0');
		LogPrintf(LOG_DEBUG, "SP = %04X   \tS  = %c", regSP, GetFlag(FLAG_S)?'1':'0');
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

	void CPU8080::interrupt(InterruptSource source)
	{
		if (m_interrupts.IsInterrupting(source))
		{
			m_interruptsEnabled = false;

			regSP--;
			m_memory.Write8(regSP, GetHByte(m_programCounter));
			regSP--;
			m_memory.Write8(regSP, GetLByte(m_programCounter));

			ADDRESS vector = 0;

			switch (source)
			{
			case TRAP:	vector = 0x24; break;
			case RST75:	vector = 0x3C; break;
			case RST65:	vector = 0x34; break;
			case RST55:	vector = 0x2C; break;
			default:
				LogPrintf(LOG_ERROR, "Invalid interrupt source");
			}

			m_programCounter = vector;
		}
	}

	void CPU8080::adjustParity(BYTE data)
	{
		SetFlag(FLAG_P, IsParityEven(data));
	}

	void CPU8080::adjustSign(BYTE data)
	{
		SetFlag(FLAG_S, (data&128)?true:false);
	}

	void CPU8080::adjustZero(BYTE data)
	{
		SetFlag(FLAG_Z, (data==0));
	}

	void CPU8080::LXIb()
	{
		regC = FetchByte();
		regB = FetchByte();
	}

	void CPU8080::LXId()
	{
		regE = FetchByte();
		regD = FetchByte();
	}

	void CPU8080::LXIh()
	{
		regL = FetchByte();
		regH = FetchByte();
	}

	void CPU8080::STAXb()
	{
		m_memory.Write8(MakeWord(regB, regC), regA);
	}

	void CPU8080::STAXd()
	{
		m_memory.Write8(MakeWord(regD, regE), regA);
	}

	void CPU8080::LDAXb()
	{
		regA = m_memory.Read8(MakeWord(regB, regC));
	}

	void CPU8080::LDAXd()
	{
		regA = m_memory.Read8(MakeWord(regD, regE));
	}

	void CPU8080::STA()
	{
		ADDRESS dest = FetchWord();
		m_memory.Write8(dest, regA);
	}

	void CPU8080::LDA()
	{
		ADDRESS src = FetchWord();
		regA = m_memory.Read8(src);
	}

	void CPU8080::SHLD()
	{
		ADDRESS dest = FetchWord();
		m_memory.Write16(dest, GetHL());
	}

	void CPU8080::LHLD()
	{
		ADDRESS src = FetchWord();
		regL = m_memory.Read8(src);
		regH = m_memory.Read8(src + 1);
	}

	void CPU8080::XCHG()
	{
		BYTE oldH, oldL;

		oldL = regL; oldH = regH;
	
		regL = regE; regH = regD;
		regE = oldL; regD = oldH;
	}

	void CPU8080::push(BYTE h, BYTE l)
	{
		m_memory.Write8(--regSP, h);
		m_memory.Write8(--regSP, l);
	}

	void CPU8080::pop(BYTE &h, BYTE &l)
	{
		l = m_memory.Read8(regSP++);
		h = m_memory.Read8(regSP++);
	}

	void CPU8080::XTHL()
	{
		BYTE oldH, oldL;

		oldL = regL; oldH = regH;
	
		regL = m_memory.Read8(regSP);
		regH = m_memory.Read8(regSP+1);
	
		m_memory.Write8(regSP, oldL);
		m_memory.Write8(regSP+1, oldH);
	}

	void CPU8080::SPHL()
	{
		regSP = GetHL();
	}

	void CPU8080::LXIsp()
	{
		regSP = FetchWord();
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
			regSP--;
			m_memory.Write8(regSP, GetHByte(m_programCounter));
			regSP--;
			m_memory.Write8(regSP, GetLByte(m_programCounter));

			m_programCounter = dest;
			TICKT3();
		}
	}

	void CPU8080::retIF(bool condition)
	{
		if (condition == true)
		{
			BYTE valL, valH;

			valL = m_memory.Read8(regSP);
			regSP++;
			valH = m_memory.Read8(regSP);
			regSP++;		

			WORD returnTo = MakeWord(valH, valL);
			m_programCounter = returnTo;
			TICKT3();
		}
	}

	void CPU8080::RST(BYTE rst)
	{
		m_memory.Write8(--regSP, GetHByte(m_programCounter));
		m_memory.Write8(--regSP, GetLByte(m_programCounter));

		m_programCounter = (rst << 3);
	}

	void CPU8080::INRr(BYTE& reg)
	{
		reg++;

		adjustParity(reg);
		adjustZero(reg);
		adjustSign(reg);
		SetFlag(FLAG_AC, (reg & 0x0F) == 0);
	}

	void CPU8080::DCRr(BYTE& reg)
	{
		reg--;

		adjustParity(reg);
		adjustZero(reg);
		adjustSign(reg);
		SetFlag(FLAG_AC, (reg & 0x0F) != 0x0F);
	}

	void CPU8080::INRm()
	{
		BYTE value = ReadMem();
		value++;
		WriteMem(value);

		adjustParity(value);
		adjustZero(value);
		adjustSign(value);
		SetFlag(FLAG_AC, (value & 0x0F) == 0);
	}

	void CPU8080::DCRm()
	{
		BYTE value = ReadMem();
		value--;
		WriteMem(value);

		adjustParity(value);
		adjustZero(value);
		adjustSign(value);
		SetFlag(FLAG_AC, (value & 0x0F) != 0x0F);
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
		BYTE loNibble = (regA & 0x0F) + (src & 0x0F);

		WORD temp = regA + src;
		if (carry)
		{
			temp++;
			loNibble++;
		}

		regA = (BYTE)temp;

		SetFlag(FLAG_CY, (temp > 0xFF));
		SetFlag(FLAG_AC, (loNibble > 0x0F));
		adjustSign(regA);
		adjustZero(regA);
		adjustParity(regA);
	}

	void CPU8080::dad(WORD value)
	{
		DWORD hl = GetHL();

		hl += value;

		SetFlag(FLAG_CY, hl > 0xFFFF);

		regH = GetHByte((WORD)hl);
		regL = GetLByte((WORD)hl);
	}

	void CPU8080::sub(BYTE src, bool borrow)
	{
		// AC flag
		char loNibble = (regA & 0x0F) - (src & 0x0F);

		int temp = regA - src;
		if (borrow)
		{
			temp--;
			loNibble--;
		}

		regA = (BYTE)temp;

		SetFlag(FLAG_CY, (temp < 0));
		SetFlag(FLAG_AC, !(loNibble < 0));
		adjustSign(regA);
		adjustZero(regA);
		adjustParity(regA);
	}

	void CPU8080::ana(BYTE src)
	{
		regA &= src;

		adjustSign(regA);
		adjustZero(regA);
		adjustParity(regA);
		SetFlag(FLAG_AC, true);				// Must confirm
		SetFlag(FLAG_CY, false);
	}

	void CPU8080::xra(BYTE src)
	{
		regA ^= src;

		adjustSign(regA);
		adjustZero(regA);
		adjustParity(regA);
		SetFlag(FLAG_AC, false);
		SetFlag(FLAG_CY, false);
	}

	void CPU8080::ora(BYTE src)
	{
		regA |= src;

		adjustSign(regA);
		adjustZero(regA);
		adjustParity(regA);
		SetFlag(FLAG_AC, false);
		SetFlag(FLAG_CY, false);
	}

	void CPU8080::cmp(BYTE src)
	{
		// AC flag
		char loNibble = (regA & 0x0F) - (src & 0x0F);

		int temp = regA - src;

		SetFlag(FLAG_CY, (temp < 0));
		SetFlag(FLAG_AC, !(loNibble < 0));
		adjustSign((BYTE)temp);
		adjustZero((BYTE)temp);
		adjustParity((BYTE)temp);
	}

	void CPU8080::RLC()
	{
		bool msb = (regA & 128);

		regA = (regA << 1);
		regA |= (msb ? 1 : 0);

		SetFlag(FLAG_CY, msb);
	}

	void CPU8080::RRC()
	{
		bool lsb = (regA & 1);

		regA = (regA >> 1);
		regA &= ~128;
		regA |= (lsb ? 128 : 0);

		SetFlag(FLAG_CY, lsb);
	}

	void CPU8080::RAL()
	{
		bool msb = (regA & 128);

		regA = (regA << 1);
		regA |= (GetFlag(FLAG_CY) ? 1 : 0);

		SetFlag(FLAG_CY, msb);
	}

	void CPU8080::RAR()
	{
		bool lsb = (regA & 1);

		regA = (regA >> 1);
		regA &= ~128;
		regA |= (GetFlag(FLAG_CY) ? 128 : 0);

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

	void CPU8080::NOP()
	{
	}

	void CPU8080::HLT()
	{
		LogPrintf(LOG_INFO, "HLT");
		m_state = CPUState::STOP;
	}
}
