#include "stdafx.h"
#include "CPU8080.h"

CPU8080::CPU8080(Memory & memory, MemoryMap &mmap, Interrupts & interrupts)
	:	CPU(memory, mmap), Logger("CPU8088"), m_interrupts(interrupts), m_interruptsEnabled(false)
{
	// -------------------
	// 1. Data Transfer Group

	// MOV r1,r2 (Move Register)
	// (r1) <- (r2)

	// Destination B
	AddOpcode(0100, (OPCodeFunction)(&CPU8080::MOVrr));	// B,B
	AddOpcode(0101, (OPCodeFunction)(&CPU8080::MOVrr));	// B,C
	AddOpcode(0102, (OPCodeFunction)(&CPU8080::MOVrr));	// B,D
	AddOpcode(0103, (OPCodeFunction)(&CPU8080::MOVrr));	// B,E
	AddOpcode(0104, (OPCodeFunction)(&CPU8080::MOVrr));	// B,H
	AddOpcode(0105, (OPCodeFunction)(&CPU8080::MOVrr));	// B,L
	AddOpcode(0107, (OPCodeFunction)(&CPU8080::MOVrr));	// B,A

	// Destination C
	AddOpcode(0110, (OPCodeFunction)(&CPU8080::MOVrr));	// C,B
	AddOpcode(0111, (OPCodeFunction)(&CPU8080::MOVrr));	// C,C
	AddOpcode(0112, (OPCodeFunction)(&CPU8080::MOVrr));	// C,D
	AddOpcode(0113, (OPCodeFunction)(&CPU8080::MOVrr));	// C,E
	AddOpcode(0114, (OPCodeFunction)(&CPU8080::MOVrr));	// C,H
	AddOpcode(0115, (OPCodeFunction)(&CPU8080::MOVrr));	// C,L
	AddOpcode(0117, (OPCodeFunction)(&CPU8080::MOVrr));	// C,A

	// Destination D
	AddOpcode(0120, (OPCodeFunction)(&CPU8080::MOVrr));	// D,B
	AddOpcode(0121, (OPCodeFunction)(&CPU8080::MOVrr));	// D,C
	AddOpcode(0122, (OPCodeFunction)(&CPU8080::MOVrr));	// D,D
	AddOpcode(0123, (OPCodeFunction)(&CPU8080::MOVrr));	// D,E
	AddOpcode(0124, (OPCodeFunction)(&CPU8080::MOVrr));	// D,H
	AddOpcode(0125, (OPCodeFunction)(&CPU8080::MOVrr));	// D,L
	AddOpcode(0127, (OPCodeFunction)(&CPU8080::MOVrr));	// D,A

	// Destination E	
	AddOpcode(0130, (OPCodeFunction)(&CPU8080::MOVrr));	// E,B
	AddOpcode(0131, (OPCodeFunction)(&CPU8080::MOVrr));	// E,C
	AddOpcode(0132, (OPCodeFunction)(&CPU8080::MOVrr));	// E,D
	AddOpcode(0133, (OPCodeFunction)(&CPU8080::MOVrr));	// E,E
	AddOpcode(0134, (OPCodeFunction)(&CPU8080::MOVrr));	// E,H
	AddOpcode(0135, (OPCodeFunction)(&CPU8080::MOVrr));	// E,L
	AddOpcode(0137, (OPCodeFunction)(&CPU8080::MOVrr));	// E,A
	
	// Destination H
	AddOpcode(0140, (OPCodeFunction)(&CPU8080::MOVrr));	// H,B
	AddOpcode(0141, (OPCodeFunction)(&CPU8080::MOVrr));	// H,C
	AddOpcode(0142, (OPCodeFunction)(&CPU8080::MOVrr));	// H,D
	AddOpcode(0143, (OPCodeFunction)(&CPU8080::MOVrr));	// H,E
	AddOpcode(0144, (OPCodeFunction)(&CPU8080::MOVrr));	// H,H
	AddOpcode(0145, (OPCodeFunction)(&CPU8080::MOVrr));	// H,L
	AddOpcode(0147, (OPCodeFunction)(&CPU8080::MOVrr));	// H,A

	// Destination L
	AddOpcode(0150, (OPCodeFunction)(&CPU8080::MOVrr));	// L,B
	AddOpcode(0151, (OPCodeFunction)(&CPU8080::MOVrr));	// L,C
	AddOpcode(0152, (OPCodeFunction)(&CPU8080::MOVrr));	// L,D
	AddOpcode(0153, (OPCodeFunction)(&CPU8080::MOVrr));	// L,E
	AddOpcode(0154, (OPCodeFunction)(&CPU8080::MOVrr));	// L,H
	AddOpcode(0155, (OPCodeFunction)(&CPU8080::MOVrr));	// L,L
	AddOpcode(0157, (OPCodeFunction)(&CPU8080::MOVrr));	// L,A

	// Destination A
	AddOpcode(0170, (OPCodeFunction)(&CPU8080::MOVrr));	// A,B
	AddOpcode(0171, (OPCodeFunction)(&CPU8080::MOVrr));	// A,C
	AddOpcode(0172, (OPCodeFunction)(&CPU8080::MOVrr));	// A,D
	AddOpcode(0173, (OPCodeFunction)(&CPU8080::MOVrr));	// A,E
	AddOpcode(0174, (OPCodeFunction)(&CPU8080::MOVrr));	// A,H
	AddOpcode(0175, (OPCodeFunction)(&CPU8080::MOVrr));	// A,L
	AddOpcode(0177, (OPCodeFunction)(&CPU8080::MOVrr));	// A,A

	// MOV r, M (Move from memory)
	// (r) <- ((H)(L))
	AddOpcode(0106, (OPCodeFunction)(&CPU8080::MOVrm));	// B,m
	AddOpcode(0116, (OPCodeFunction)(&CPU8080::MOVrm));	// C,m
	AddOpcode(0126, (OPCodeFunction)(&CPU8080::MOVrm));	// D,m
	AddOpcode(0136, (OPCodeFunction)(&CPU8080::MOVrm));	// E,m
	AddOpcode(0146, (OPCodeFunction)(&CPU8080::MOVrm));	// H,m
	AddOpcode(0156, (OPCodeFunction)(&CPU8080::MOVrm));	// L,m
	AddOpcode(0176, (OPCodeFunction)(&CPU8080::MOVrm));	// A,m

	// MOV M, r (Move to memory)
	// ((H)(L) <- (r)
	AddOpcode(0160, (OPCodeFunction)(&CPU8080::MOVmr));	// m,B
	AddOpcode(0161, (OPCodeFunction)(&CPU8080::MOVmr));	// m,C
	AddOpcode(0162, (OPCodeFunction)(&CPU8080::MOVmr));	// m,D
	AddOpcode(0163, (OPCodeFunction)(&CPU8080::MOVmr));	// m,E
	AddOpcode(0164, (OPCodeFunction)(&CPU8080::MOVmr));	// m,H
	AddOpcode(0165, (OPCodeFunction)(&CPU8080::MOVmr));	// m,L
	AddOpcode(0167, (OPCodeFunction)(&CPU8080::MOVmr));	// m,A

	// MVI r, data (Move immediate)
	// (r) <- (byte 2)
	AddOpcode(0006, (OPCodeFunction)(&CPU8080::MVIr));	// B
	AddOpcode(0016, (OPCodeFunction)(&CPU8080::MVIr));	// C
	AddOpcode(0026, (OPCodeFunction)(&CPU8080::MVIr));	// D
	AddOpcode(0036, (OPCodeFunction)(&CPU8080::MVIr));	// E
	AddOpcode(0046, (OPCodeFunction)(&CPU8080::MVIr));	// H
	AddOpcode(0056, (OPCodeFunction)(&CPU8080::MVIr));	// L
	AddOpcode(0076, (OPCodeFunction)(&CPU8080::MVIr));	// A

	// MVI M, data (Move to memory immediate)
	// ((H)(L) <- (byte 2)
	AddOpcode(0066, (OPCodeFunction)(&CPU8080::MVIm));	// m

	// LXI rp, data 16 (Load register pair immediate)
	// (rh) <- (byte3)
	// (lh) <- (byte2)
	AddOpcode(0001, (OPCodeFunction)(&CPU8080::LXIb));	// BC
	AddOpcode(0021, (OPCodeFunction)(&CPU8080::LXId));	// DE
	AddOpcode(0041, (OPCodeFunction)(&CPU8080::LXIh));	// HL
	AddOpcode(0061, (OPCodeFunction)(&CPU8080::LXIsp));	// SP

	// LDA addr (Load Accumulator direct)
	// (A) <- ((byte2)(byte3))
	AddOpcode(0072, (OPCodeFunction)(&CPU8080::LDA));

	// STA addr (Store Accumulator direct)
	// ((byte2)(byte3)) <- (A)
	AddOpcode(0062, (OPCodeFunction)(&CPU8080::STA));

	// LHLD addr (Load H and L direct)
	// (H)<-((byte2)(byte3))
	// (L)<-((byte2)(byte3)+1)
	AddOpcode(0052, (OPCodeFunction)(&CPU8080::LHLD));

	// SHLD addr (Store H and L direct)
	// ((byte2)(byte3))<-(H)
	// ((byte2)(byte3)+1)<-(L)
	AddOpcode(0042, (OPCodeFunction)(&CPU8080::SHLD));

	// LDAX rp (Load accumulator indirect)
	// (A) <- ((rp))
	// Note: only register pairs rp=B (B and C) or rp=D (D and E) 
	// may be specified.
	AddOpcode(0012, (OPCodeFunction)(&CPU8080::LDAXb));
	AddOpcode(0032, (OPCodeFunction)(&CPU8080::LDAXd));

	// STAX rp (Store accumulator indirect)
	// ((rp) <- (A)
	// Note: only register pairs rp=B (B and C) or rp=D (D and E) 
	// may be specified.
	AddOpcode(0002, (OPCodeFunction)(&CPU8080::STAXb));
	AddOpcode(0022, (OPCodeFunction)(&CPU8080::STAXd));

	// XCHG (Exchange H and L with D and E)
	// (H) <- (D)
	// (L) <- (E)
	AddOpcode(0353, (OPCodeFunction)(&CPU8080::XCHG));
	
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
	AddOpcode(0200, (OPCodeFunction)(&CPU8080::ADDr));	// B
	AddOpcode(0201, (OPCodeFunction)(&CPU8080::ADDr));	// C
	AddOpcode(0202, (OPCodeFunction)(&CPU8080::ADDr));	// D
	AddOpcode(0203, (OPCodeFunction)(&CPU8080::ADDr));	// E
	AddOpcode(0204, (OPCodeFunction)(&CPU8080::ADDr));	// H
	AddOpcode(0205, (OPCodeFunction)(&CPU8080::ADDr));	// L
	AddOpcode(0207, (OPCodeFunction)(&CPU8080::ADDr));	// A

	// ADD M (Add memory)
	// (A) <- (A) + ((H)(L))
	AddOpcode(0206, (OPCodeFunction)(&CPU8080::ADDm));	// m

	// ADI data (Add immediate)
	// (A) <- (A) + (byte2)
	AddOpcode(0306, (OPCodeFunction)(&CPU8080::ADI));

	// ADC r (Add Register with Carry)
	// (A) <- (A) + (r) + (CY)
	AddOpcode(0210, (OPCodeFunction)(&CPU8080::ADCr));	// B
	AddOpcode(0211, (OPCodeFunction)(&CPU8080::ADCr));	// C
	AddOpcode(0212, (OPCodeFunction)(&CPU8080::ADCr));	// D
	AddOpcode(0213, (OPCodeFunction)(&CPU8080::ADCr));	// E
	AddOpcode(0214, (OPCodeFunction)(&CPU8080::ADCr));	// H
	AddOpcode(0215, (OPCodeFunction)(&CPU8080::ADCr));	// L
	AddOpcode(0217, (OPCodeFunction)(&CPU8080::ADCr));	// A

	// ADC M (Add Memory with Carry)
	// (A) <- (A) + ((H)(L)) + (CY)
	AddOpcode(0216, (OPCodeFunction)(&CPU8080::ADCm));	// m

	// ACI data (Add immediate with Carry)
	// (A) <- (A) + (byte2) + (CY)
	AddOpcode(0316, (OPCodeFunction)(&CPU8080::ACI));

	// SUB r (Substract Register)
	// (A) <- (A) - (r)
	AddOpcode(0220, (OPCodeFunction)(&CPU8080::SUBr));	// B
	AddOpcode(0221, (OPCodeFunction)(&CPU8080::SUBr));	// C
	AddOpcode(0222, (OPCodeFunction)(&CPU8080::SUBr));	// D
	AddOpcode(0223, (OPCodeFunction)(&CPU8080::SUBr));	// E
	AddOpcode(0224, (OPCodeFunction)(&CPU8080::SUBr));	// H
	AddOpcode(0225, (OPCodeFunction)(&CPU8080::SUBr));	// L
	AddOpcode(0227, (OPCodeFunction)(&CPU8080::SUBr));	// A

	// SUB M (Substract memory)
	// (A) <- (A) - ((H)(L))
	AddOpcode(0226, (OPCodeFunction)(&CPU8080::SUBm));	// m

	// SUI data (Substract immediate)
	// (A) <- (A) - (byte2)
	AddOpcode(0326, (OPCodeFunction)(&CPU8080::SUI));

	// SBB r (Substract Register with Borrow)
	// (A) <- (A) - (r) - (CY)
	AddOpcode(0230, (OPCodeFunction)(&CPU8080::SBBr));	// B
	AddOpcode(0231, (OPCodeFunction)(&CPU8080::SBBr));	// C
	AddOpcode(0232, (OPCodeFunction)(&CPU8080::SBBr));	// D
	AddOpcode(0233, (OPCodeFunction)(&CPU8080::SBBr));	// E
	AddOpcode(0234, (OPCodeFunction)(&CPU8080::SBBr));	// H
	AddOpcode(0235, (OPCodeFunction)(&CPU8080::SBBr));	// L
	AddOpcode(0237, (OPCodeFunction)(&CPU8080::SBBr));	// A

	// SBB M (Substract Memory with Borrow)
	// (A) <- (A) - ((H)(L)) - (CY)
	AddOpcode(0236, (OPCodeFunction)(&CPU8080::SBBm));	// m

	// SBI data (Substract immediate with Borrow)
	// (A) <- (A) - (byte2) - (CY)
	AddOpcode(0336, (OPCodeFunction)(&CPU8080::SBI));

	// INR r (Increment Register)
	// (r) <- (r) + 1
	// Note: CY not affected
	AddOpcode(0004, (OPCodeFunction)(&CPU8080::INRr));	// B
	AddOpcode(0014, (OPCodeFunction)(&CPU8080::INRr));	// C
	AddOpcode(0024, (OPCodeFunction)(&CPU8080::INRr));	// D
	AddOpcode(0034, (OPCodeFunction)(&CPU8080::INRr));	// E
	AddOpcode(0044, (OPCodeFunction)(&CPU8080::INRr));	// H
	AddOpcode(0054, (OPCodeFunction)(&CPU8080::INRr));	// L
	AddOpcode(0074, (OPCodeFunction)(&CPU8080::INRr));	// A

	// INR M (Increment Memory)
	// ((H)(L)) <- ((H)(L)) + 1
	// Note: CY not affected
	AddOpcode(0064, (OPCodeFunction)(&CPU8080::INRm));	// m

	// DCR r (Decrement Register)
	// (r) <- (r) - 1
	// Note: CY not affected
	AddOpcode(0005, (OPCodeFunction)(&CPU8080::DCRr));	// B
	AddOpcode(0015, (OPCodeFunction)(&CPU8080::DCRr));	// C
	AddOpcode(0025, (OPCodeFunction)(&CPU8080::DCRr));	// D
	AddOpcode(0035, (OPCodeFunction)(&CPU8080::DCRr));	// E
	AddOpcode(0045, (OPCodeFunction)(&CPU8080::DCRr));	// H
	AddOpcode(0055, (OPCodeFunction)(&CPU8080::DCRr));	// L
	AddOpcode(0075, (OPCodeFunction)(&CPU8080::DCRr));	// A
	
	// DCR M (Decrement Memory)
	// ((H)(L)) <- ((H)(L)) - 1
	// Note: CY not affected
	AddOpcode(0065, (OPCodeFunction)(&CPU8080::DCRm));	// m

	// INX rp (Increment Register Pair)
	// (rh)(rl) <- (rh)(rl) + 1
	// Note: No condition flags are affected
	AddOpcode(0003, (OPCodeFunction)(&CPU8080::INXb));
	AddOpcode(0023, (OPCodeFunction)(&CPU8080::INXd));
	AddOpcode(0043, (OPCodeFunction)(&CPU8080::INXh));
	AddOpcode(0063, (OPCodeFunction)(&CPU8080::INXsp));

	// DCX rp (Decrement Register Pair)
	// (rh)(rl) <- (rh)(rl) - 1
	// Note: No condition flags are affected
	AddOpcode(0013, (OPCodeFunction)(&CPU8080::DCXb));
	AddOpcode(0033, (OPCodeFunction)(&CPU8080::DCXd));
	AddOpcode(0053, (OPCodeFunction)(&CPU8080::DCXh));
	AddOpcode(0073, (OPCodeFunction)(&CPU8080::DCXsp));

	// DAD rp (Add register pair to H and L)
	// (H)(L) <- (H)(L) + (rh)(rl)
	// Note: Only the CY flag is affected
	AddOpcode(0011, (OPCodeFunction)(&CPU8080::DADb));
	AddOpcode(0031, (OPCodeFunction)(&CPU8080::DADd));
	AddOpcode(0051, (OPCodeFunction)(&CPU8080::DADh));
	AddOpcode(0071, (OPCodeFunction)(&CPU8080::DADsp));

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
	//	AddOpcode(0047, (OPCodeFunction)(&CPU8080::DAA)); // TODO: Not implemented
	AddOpcode(0047, (OPCodeFunction)(&CPU8080::DAA)); // TODO: Currently used to dump regs

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
	AddOpcode(0240, (OPCodeFunction)(&CPU8080::ANAr));	// B
	AddOpcode(0241, (OPCodeFunction)(&CPU8080::ANAr));	// C
	AddOpcode(0242, (OPCodeFunction)(&CPU8080::ANAr));	// D
	AddOpcode(0243, (OPCodeFunction)(&CPU8080::ANAr));	// E
	AddOpcode(0244, (OPCodeFunction)(&CPU8080::ANAr));	// H
	AddOpcode(0245, (OPCodeFunction)(&CPU8080::ANAr));	// L
	AddOpcode(0247, (OPCodeFunction)(&CPU8080::ANAr));	// A

	// ANA M (AND Memory)
	// (A) <- (A) and ((H)(L))
	// Note: same CY rules as ANA
	AddOpcode(0246, (OPCodeFunction)(&CPU8080::ANAm));	// m

	// ANI data (AND Immediate)
	// Note: same CY rules as ANA
	// (A) <- (A) and (byte 2)
	AddOpcode(0346, (OPCodeFunction)(&CPU8080::ANI));

	// XRA r (XOR Register)
	// (A) <- (A) xor (r)
	// Note: The CY  and AC flags are cleared
	AddOpcode(0250, (OPCodeFunction)(&CPU8080::XRAr));	// B
	AddOpcode(0251, (OPCodeFunction)(&CPU8080::XRAr));	// C
	AddOpcode(0252, (OPCodeFunction)(&CPU8080::XRAr));	// D
	AddOpcode(0253, (OPCodeFunction)(&CPU8080::XRAr));	// E
	AddOpcode(0254, (OPCodeFunction)(&CPU8080::XRAr));	// H
	AddOpcode(0255, (OPCodeFunction)(&CPU8080::XRAr));	// L
	AddOpcode(0257, (OPCodeFunction)(&CPU8080::XRAr));	// A

	// XRA M (XOR Memory)
	// (A) <- (A) xor ((H)(L))
	// Note: The CY  and AC flags are cleared
	AddOpcode(0256, (OPCodeFunction)(&CPU8080::XRAm));	// m

	// XRI data (XOR Immediate)
	// (A) <- (A) xor (byte 2)
	// Note: The CY  and AC flags are cleared
	AddOpcode(0356, (OPCodeFunction)(&CPU8080::XRI));

	// ORA r (OR Register)
	// (A) <- (A) or (r)
	// Note: The CY  and AC flags are cleared
	AddOpcode(0260, (OPCodeFunction)(&CPU8080::ORAr));	// B
	AddOpcode(0261, (OPCodeFunction)(&CPU8080::ORAr));	// C
	AddOpcode(0262, (OPCodeFunction)(&CPU8080::ORAr));	// D
	AddOpcode(0263, (OPCodeFunction)(&CPU8080::ORAr));	// E
	AddOpcode(0264, (OPCodeFunction)(&CPU8080::ORAr));	// H
	AddOpcode(0265, (OPCodeFunction)(&CPU8080::ORAr));	// L
	AddOpcode(0267, (OPCodeFunction)(&CPU8080::ORAr));	// A

	// ORA M (OR Memory)
	// (A) <- (A) or ((H)(L))
	// Note: The CY  and AC flags are cleared
	AddOpcode(0266, (OPCodeFunction)(&CPU8080::ORAm));	// m

	// ORI data (OR Immediate)
	// (A) <- (A) oor (byte 2)
	// Note: The CY  and AC flags are cleared
	AddOpcode(0366, (OPCodeFunction)(&CPU8080::ORI));

	// CMP r (Compare Register)
	// (A) - (r)
	// Note: The accumulator remains unchanged.
	// The condition flags are set are set as
	// the result of the substraction.  The Z flag
	// is set to 1 if (A) = (r).  The CY flag is set
	// to 1 if (A) < (r)
	AddOpcode(0270, (OPCodeFunction)(&CPU8080::CMPr));	// B
	AddOpcode(0271, (OPCodeFunction)(&CPU8080::CMPr));	// C
	AddOpcode(0272, (OPCodeFunction)(&CPU8080::CMPr));	// D
	AddOpcode(0273, (OPCodeFunction)(&CPU8080::CMPr));	// E
	AddOpcode(0274, (OPCodeFunction)(&CPU8080::CMPr));	// H
	AddOpcode(0275, (OPCodeFunction)(&CPU8080::CMPr));	// L
	AddOpcode(0277, (OPCodeFunction)(&CPU8080::CMPr));	// A

	// CMP M (Compare Memory)
	// (A) - ((H)(L))
	// Note: Same rules as CMP r
	AddOpcode(0276, (OPCodeFunction)(&CPU8080::CMPm));	// m

	// CPI data (Compare immediate)
	// (A) - (byte2)
	// Note Same rules as CMP r
	AddOpcode(0376, (OPCodeFunction)(&CPU8080::CPI));

	// RLC (Rotate left)
	// (An+1) <- (An)
	// (A0) <- (A7)
	// (CY) <- (A7)
	// Note: Only the CY flag is affected
	AddOpcode(0007, (OPCodeFunction)(&CPU8080::RLC));

	// RRC (Rotate right)
	// (An) <- (An+1)
	// (A7) <- (A0)
	// (CY) <- (A0)
	// Note: Only the CY flag is affected
	AddOpcode(0017, (OPCodeFunction)(&CPU8080::RRC));

	// RAL (Rotate left through carry)
	// (An+1) <- (An)
	// (CY) <- (A7)
	// (A0) <- (CY)
	// Note: Only the CY flag is affected
	AddOpcode(0027, (OPCodeFunction)(&CPU8080::RAL));

	// RAL (Rotate right through carry)
	// (An) <- (An+1)
	// (CY) <- (A0)
	// (A7) <- (CY)
	// Note: Only the CY flag is affected
	AddOpcode(0037, (OPCodeFunction)(&CPU8080::RAR));

	// CMA (Complement accumulator)
	// (A) <- /(A)
	// Note: No flags are affected
	AddOpcode(0057, (OPCodeFunction)(&CPU8080::CMA));

	// CMC (Complement carry)
	// (CY) <- /(CY)
	// Note: No other flags are affected
	AddOpcode(0077, (OPCodeFunction)(&CPU8080::CMC));
	
	// STC (Set Carry)
	// (CY) <- 1
	// Note: No other flags are affected
	AddOpcode(0067, (OPCodeFunction)(&CPU8080::STC));

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
	AddOpcode(0303, (OPCodeFunction)(&CPU8080::JMP));

	// Jcondition addr (Conditional Jump)
	// If (CCC),
	// (PC) <- (byte3)(byte2)
	AddOpcode(0302, (OPCodeFunction)(&CPU8080::JNZ));
	AddOpcode(0312, (OPCodeFunction)(&CPU8080::JZ));
	AddOpcode(0322, (OPCodeFunction)(&CPU8080::JNC));
	AddOpcode(0332, (OPCodeFunction)(&CPU8080::JC));
	AddOpcode(0342, (OPCodeFunction)(&CPU8080::JPO));
	AddOpcode(0352, (OPCodeFunction)(&CPU8080::JPE));
	AddOpcode(0362, (OPCodeFunction)(&CPU8080::JP));
	AddOpcode(0372, (OPCodeFunction)(&CPU8080::JM));

	// CALL addr (Call)
	// ((SP) - 1) <- (PCH)
	// ((SP) - 2) <- (PCL)
	// (SP) <- (SP) - 2
	// (PC) <- (byte3)(byte2)
	AddOpcode(0315, (OPCodeFunction)(&CPU8080::CALL));

	// Ccondition addr (Conditional Call)
	// If (CCC),
	// ((SP) - 1) <- (PCH)
	// ((SP) - 2) <- (PCL)
	// (SP) <- (SP) - 2
	// (PC) <- (byte3)(byte2)
	AddOpcode(0304, (OPCodeFunction)(&CPU8080::CNZ));
	AddOpcode(0314, (OPCodeFunction)(&CPU8080::CZ));
	AddOpcode(0324, (OPCodeFunction)(&CPU8080::CNC));
	AddOpcode(0334, (OPCodeFunction)(&CPU8080::CC));
	AddOpcode(0344, (OPCodeFunction)(&CPU8080::CPO));
	AddOpcode(0354, (OPCodeFunction)(&CPU8080::CPE));
	AddOpcode(0364, (OPCodeFunction)(&CPU8080::CP));
	AddOpcode(0374, (OPCodeFunction)(&CPU8080::CM));

	// RET (Return)
	// (PCL) <- ((SP))
	// (PCH) <- ((SP) + 1)
	// (SP) <- (SP) + 2
	AddOpcode(0311, (OPCodeFunction)(&CPU8080::RET));

	// Rcondition (Conditional Return)
	// If (CCC),
	// (PCL) <- ((SP))
	// (PCH) <- ((SP) + 1)
	// (SP) <- (SP) + 2
	AddOpcode(0300, (OPCodeFunction)(&CPU8080::RNZ));
	AddOpcode(0310, (OPCodeFunction)(&CPU8080::RZ));
	AddOpcode(0320, (OPCodeFunction)(&CPU8080::RNC));
	AddOpcode(0330, (OPCodeFunction)(&CPU8080::RC));
	AddOpcode(0340, (OPCodeFunction)(&CPU8080::RPO));
	AddOpcode(0350, (OPCodeFunction)(&CPU8080::RPE));
	AddOpcode(0360, (OPCodeFunction)(&CPU8080::RP));
	AddOpcode(0370, (OPCodeFunction)(&CPU8080::RM));

	// RST n (Restart)
	// ((SP) - 1) <- (PCH)
	// ((SP) - 2) <- (PCL)
	// (SP) <- (SP) - 2
	// (PC) <- 8 * NNN
	AddOpcode(0307, (OPCodeFunction)(&CPU8080::RST));
	AddOpcode(0317, (OPCodeFunction)(&CPU8080::RST));
	AddOpcode(0327, (OPCodeFunction)(&CPU8080::RST));
	AddOpcode(0337, (OPCodeFunction)(&CPU8080::RST));
	AddOpcode(0347, (OPCodeFunction)(&CPU8080::RST));
	AddOpcode(0357, (OPCodeFunction)(&CPU8080::RST));
	AddOpcode(0367, (OPCodeFunction)(&CPU8080::RST));
	AddOpcode(0377, (OPCodeFunction)(&CPU8080::RST));

	// PCHL (Jump H and L indirect - move H and L to PC)
	// (PCH)<-(H)
	// (PCL)<-(L)
	AddOpcode(0351, (OPCodeFunction)(&CPU8080::PCHL));

	// -------------------
	// 5. Stack, I/O and Machine Control Group

	// PUSH rp (Push)
	// ((SP) - 1)<-(rh)
	// ((SP) - 2)<-(rl)
	// (SP)<-(SP)-2
	// Note: Register pair rp=SP may not be specified
	AddOpcode(0305, (OPCodeFunction)(&CPU8080::PUSHb));
	AddOpcode(0325, (OPCodeFunction)(&CPU8080::PUSHd));
	AddOpcode(0345, (OPCodeFunction)(&CPU8080::PUSHh));

	// PUSH PSW (Push processor status word)
	// ((SP) - 1)  <- (A)
	// ((SP) - 2)0 <- (CY)
	// ((SP) - 2)1 <- (Undefined)
	// ((SP) - 2)2 <- (P)
	// ((SP) - 2)3 <- (Undefined)
	// ((SP) - 2)4 <- (AC)
	// ((SP) - 2)5 <- (Undefined)
	// ((SP) - 2)6 <- (Z)
	// ((SP) - 2)7 <- (S)
	// (SP)<-(SP)-2
	AddOpcode(0365, (OPCodeFunction)(&CPU8080::PUSHpsw));

	// POP rp (Pop)
	// (rl) <- ((SP))
	// (rh) <- ((SP) + 1)
	// (SP) <- (SP)+2
	// Note: Register pair rp=SP may not be specified
	AddOpcode(0301, (OPCodeFunction)(&CPU8080::POPb));
	AddOpcode(0321, (OPCodeFunction)(&CPU8080::POPd));
	AddOpcode(0341, (OPCodeFunction)(&CPU8080::POPh));

	// POP PSW (Pop processor status word)
	// (CY) <- ((SP))0
	// (P)  <- ((SP))2
	// (AC) <- ((SP))4
	// (Z)  <- ((SP))6
	// (S)  <- ((SP))7
	// (A)  <- ((SP)+1)
	// (SP) <- (SP)+2
	AddOpcode(0361, (OPCodeFunction)(&CPU8080::POPpsw));

	//XTHL (Exchange stack top with H and L)
	// (L) <- ((SP))
	// (H) <- ((SP) + 1)
	AddOpcode(0343, (OPCodeFunction)(&CPU8080::XTHL));

	// SPHL (Move HL to SP)
	// (SP) <- (H)(L)
	AddOpcode(0371, (OPCodeFunction)(&CPU8080::SPHL));

	// IN port (Input)
	// (A) <- (data)
	AddOpcode(0333, (OPCodeFunction)(&CPU8080::IN));

	// OUT port (Output)
	// (data) <- (A)
	AddOpcode(0323, (OPCodeFunction)(&CPU8080::OUT));

	// EI (Enable Interrupts)
	// The interrupt system is enabled following the execution
	// of the next instruction. 
	AddOpcode(0373, (OPCodeFunction)(&CPU8080::EI));

	// DI (Disable Interrupts)
	// The interrupt ststem is disabled immediately following
	// the execution of the DI instruction.  Interrupts are
	// not recognized during the DI instruction
	AddOpcode(0363, (OPCodeFunction)(&CPU8080::DI));

	// HLT (Halt)
	AddOpcode(0166, (OPCodeFunction)(&CPU8080::HLT));

	// NOP (No op)
	AddOpcode(0000, (OPCodeFunction)(&CPU8080::NOP));

	// RIM (Read Interrupt Mask) (8085)
	AddOpcode(0040, (OPCodeFunction)(&CPU8080::NOP));		//8085 TODO: RIM

	// SIM (Set Interrupt Mask) (8085)
	AddOpcode(0060, (OPCodeFunction)(&CPU8080::NOP));		// 8085 TODO: SIM
}

CPU8080::~CPU8080()
{

}

void CPU8080::AddDevice(PortConnector & ports)
{
	return m_ports.Connect(ports);
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
	flags = 0;
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
	LogPrintf(LOG_DEBUG, "AF = %02X %02X\tCY = %c\n", regA, flags, getFlag(CY_FLAG)?'1':'0');
	LogPrintf(LOG_DEBUG, "BC = %02X %02X\tP  = %c\n", regB, regC, getFlag(P_FLAG)?'1':'0');
	LogPrintf(LOG_DEBUG, "DE = %02X %02X\tAC = %c\n", regD, regE, getFlag(AC_FLAG)?'1':'0');
	LogPrintf(LOG_DEBUG, "HL = %02X %02X\tZ  = %c\n", regH, regL, getFlag(Z_FLAG)?'1':'0');
	LogPrintf(LOG_DEBUG, "SP = %04X   \tS  = %c\n", regSP, getFlag(S_FLAG)?'1':'0');
	LogPrintf(LOG_DEBUG, "PC = %04X\n", m_programCounter);
	LogPrintf(LOG_DEBUG, "\n");
}


void CPU8080::interrupt(InterruptSource source)
{
	if (m_interrupts.IsInterrupting(source))
	{
		m_interruptsEnabled = false;

		regSP--;
		m_memory.Write(regSP, getHByte(m_programCounter));
		regSP--;
		m_memory.Write(regSP, getLByte(m_programCounter));

		WORD vector;

		switch (source)
		{
		case TRAP:	vector = 0x24; break;
		case RST75:	vector = 0x3C; break;
		case RST65:	vector = 0x34; break;
		case RST55:	vector = 0x2C; break;
		default:
			LogPrintf(LOG_ERROR, "Invalid interrupt source\n");
		}

		m_timeTicks += 12;
		m_programCounter = vector;
	}
}

BYTE &CPU8080::getRegL(BYTE opcode)
{
	opcode &= 070;

	switch(opcode)
	{
	case 000:	return regB;
	case 010:	return regC;
	case 020:	return regD;
	case 030:	return regE;
	case 040:	return regH;
	case 050:	return regL;
	case 070:	return regA;

	default:
		LogPrintf(LOG_ERROR, "reg flag = mem\n");
	}

	return dummy;
}

BYTE &CPU8080::getRegR(BYTE opcode)
{
	opcode &= 007;

	switch(opcode)
	{
	case 000:	return regB;
	case 001:	return regC;
	case 002:	return regD;
	case 003:	return regE;
	case 004:	return regH;
	case 005:	return regL;
	case 007:	return regA;

	default:
		LogPrintf(LOG_ERROR, "reg flag = mem\n");
	}

	return dummy;
}

void CPU8080::adjustParity(BYTE data)
{
	setFlag(P_FLAG, isParityEven(data));
}

void CPU8080::adjustSign(BYTE data)
{
	setFlag(S_FLAG, (data&128)?true:false);
}

void CPU8080::adjustZero(BYTE data)
{
	setFlag(Z_FLAG, (data==0));
}

void CPU8080::MOVrr(BYTE opcode)
{
	getRegL(opcode) = getRegR(opcode);

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::MOVmr(BYTE opcode)
{
	m_memory.Write(getHL(), getRegR(opcode));

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::MOVrm(BYTE opcode)
{

	m_memory.Read(getHL(), getRegL(opcode));

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::MVIr(BYTE opcode)
{
	m_memory.Read(m_programCounter+1, getRegL(opcode));

	m_timeTicks += 7;
	m_programCounter+=2;
}

void CPU8080::MVIm(BYTE opcode)
{
	BYTE value;
	m_memory.Read(m_programCounter+1, value);
	m_memory.Write(getHL(), value);

	m_timeTicks += 10;
	m_programCounter+=2;
}

void CPU8080::LXIb(BYTE opcode)
{
	m_memory.Read(m_programCounter+1, regC);
	m_memory.Read(m_programCounter+2, regB);

	m_timeTicks += 10;
	m_programCounter+=3;
}

void CPU8080::LXId(BYTE opcode)
{
	m_memory.Read(m_programCounter+1, regE);
	m_memory.Read(m_programCounter+2, regD);

	m_timeTicks += 10;
	m_programCounter+=3;
}

void CPU8080::LXIh(BYTE opcode)
{
	m_memory.Read(m_programCounter+1, regL);
	m_memory.Read(m_programCounter+2, regH);

	m_timeTicks += 10;
	m_programCounter+=3;
}

void CPU8080::STAXb(BYTE opcode)
{
	m_memory.Write(getWord(regB, regC), regA);

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::STAXd(BYTE opcode)
{
	m_memory.Write(getWord(regD, regE), regA);

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::LDAXb(BYTE opcode)
{
	m_memory.Read(getWord(regB, regC), regA);
	
	m_timeTicks += 7;
	m_programCounter++;
}
void CPU8080::LDAXd(BYTE opcode)
{
	m_memory.Read(getWord(regD, regE), regA);
	
	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::STA(BYTE opcode)
{
	BYTE valH, valL;

	m_memory.Read(m_programCounter+1, valL);
	m_memory.Read(m_programCounter+2, valH);

	m_memory.Write(getWord(valH, valL), regA);

	m_timeTicks += 13;
	m_programCounter+=3;
}

void CPU8080::LDA(BYTE opcode)
{
	BYTE valH, valL;

	m_memory.Read(m_programCounter+1, valL);
	m_memory.Read(m_programCounter+2, valH);

	m_memory.Read(getWord(valH, valL), regA);

	m_timeTicks += 13;
	m_programCounter+=3;
}

void CPU8080::SHLD(BYTE opcode)
{
	BYTE valH, valL;

	m_memory.Read(m_programCounter+1, valL);
	m_memory.Read(m_programCounter+2, valH);

	m_memory.Write(getWord(valH, valL), regL);
	m_memory.Write(getWord(valH, valL)+1, regH);

	m_timeTicks += 16;
	m_programCounter+=3;
}

void CPU8080::LHLD(BYTE opcode)
{
	BYTE valH, valL;

	m_memory.Read(m_programCounter+1, valL);
	m_memory.Read(m_programCounter+2, valH);

	m_memory.Read(getWord(valH, valL), regL);
	m_memory.Read(getWord(valH, valL)+1, regH);

	m_timeTicks += 16;
	m_programCounter+=3;
}

void CPU8080::XCHG(BYTE opcode)
{
	BYTE oldH, oldL;

	oldL = regL; oldH = regH;
	
	regL = regE; regH = regD;
	regE = oldL; regD = oldH;

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::push(BYTE h, BYTE l)
{
	regSP--;
	m_memory.Write(regSP, h);
	regSP--;
	m_memory.Write(regSP, l);

	m_timeTicks += 12;
	m_programCounter++;
}

void CPU8080::PUSHb(BYTE opcode)
{
	push(regB, regC);
}

void CPU8080::PUSHd(BYTE opcode)
{
	push(regD, regE);
}

void CPU8080::PUSHh(BYTE opcode)
{
	push(regH, regL);
}

void CPU8080::PUSHpsw(BYTE opcode)
{
	push(regA, flags);
}

void CPU8080::pop(BYTE &h, BYTE &l)
{
	m_memory.Read(regSP, l);
	regSP++;
	m_memory.Read(regSP, h);
	regSP++;

	m_timeTicks += 12;
	m_programCounter++;
}

void CPU8080::POPb(BYTE opcode)
{
	pop(regB, regC);
}

void CPU8080::POPd(BYTE opcode)
{
	pop(regD, regE);
}

void CPU8080::POPh(BYTE opcode)
{
	pop(regH, regL);
}

void CPU8080::POPpsw(BYTE opcode)
{
	pop(regA, flags);
}

void CPU8080::XTHL(BYTE opcode)
{
	BYTE oldH, oldL;

	oldL = regL; oldH = regH;
	
	m_memory.Read(regSP, regL);
	m_memory.Read(regSP+1, regH);
	
	m_memory.Write(regSP, oldL);
	m_memory.Write(regSP+1, oldH);

	m_timeTicks += 16;
	m_programCounter++;
}

void CPU8080::SPHL(BYTE opcode)
{
	regSP = getHL();

	m_timeTicks += 6;
	m_programCounter++;
}

void CPU8080::LXIsp(BYTE opcode)
{
	BYTE valH, valL;

	m_memory.Read(m_programCounter+1, valL);
	m_memory.Read(m_programCounter+2, valH);

	regSP = getWord(valH, valL);

	m_timeTicks += 10;
	m_programCounter+=3;
}

void CPU8080::INXsp(BYTE opcode)
{
	regSP++;

	m_timeTicks += 6;
	m_programCounter++;
}

void CPU8080::DCXsp(BYTE opcode)
{
	regSP--;

	m_timeTicks += 6;
	m_programCounter++;
}

void CPU8080::jumpIF(bool condition, int timeT)
{
	if (condition == true)
	{
		BYTE valH, valL;

		m_memory.Read(m_programCounter+1, valL);
		m_memory.Read(m_programCounter+2, valH);

		m_timeTicks += timeT;
		m_programCounter = getWord(valH, valL);
	}
	else
	{
		m_timeTicks += timeT;
		m_programCounter += 3;
	}
}

void CPU8080::JMP(BYTE opcode)
{
	jumpIF(true, 7);
}

void CPU8080::JC(BYTE opcode)
{
	jumpIF(getFlag(CY_FLAG) == true, 10);
}

void CPU8080::JNC(BYTE opcode)
{
	jumpIF(getFlag(CY_FLAG) == false, 10);
}

void CPU8080::JZ(BYTE opcode)
{
	jumpIF(getFlag(Z_FLAG) == true, 10);
}

void CPU8080::JNZ(BYTE opcode)
{
	jumpIF(getFlag(Z_FLAG) == false, 10);
}

void CPU8080::JP(BYTE opcode)
{
	jumpIF(getFlag(S_FLAG) == false, 10);
}

void CPU8080::JM(BYTE opcode)
{
	jumpIF(getFlag(S_FLAG) == true, 10);
}

void CPU8080::JPE(BYTE opcode)
{
	jumpIF(getFlag(P_FLAG) == true, 10);
}

void CPU8080::JPO(BYTE opcode)
{
	jumpIF(getFlag(P_FLAG) == false, 10);
}

void CPU8080::PCHL(BYTE opcode)
{
	m_timeTicks += 6;
	m_programCounter = getHL();
}

void CPU8080::callIF(bool condition)
{
	if (condition == true)
	{
		WORD calledFrom = m_programCounter;
		BYTE valL, valH;
		m_memory.Read(m_programCounter+1, valL);
		m_memory.Read(m_programCounter+2, valH);
		
		regSP--;
		m_memory.Write(regSP, getHByte(m_programCounter+3));
		regSP--;
		m_memory.Write(regSP, getLByte(m_programCounter+3));

		m_timeTicks += 18;
		m_programCounter = getWord(valH, valL);

		OnCall(calledFrom, m_programCounter);
	}
	else
	{
		m_timeTicks += 9;
		m_programCounter+=3;
	}
}

void CPU8080::CALL(BYTE opcode)
{
	callIF(true);
}

void CPU8080::CC(BYTE opcode)
{
	callIF(getFlag(CY_FLAG) == true);
}

void CPU8080::CNC(BYTE opcode)
{
	callIF(getFlag(CY_FLAG) == false);
}

void CPU8080::CZ(BYTE opcode)
{
	callIF(getFlag(Z_FLAG) == true);
}

void CPU8080::CNZ(BYTE opcode)
{
	callIF(getFlag(Z_FLAG) == false);
}

void CPU8080::CP(BYTE opcode)
{
	callIF(getFlag(S_FLAG) == false);
}

void CPU8080::CM(BYTE opcode)
{
	callIF(getFlag(S_FLAG) == true);
}

void CPU8080::CPE(BYTE opcode)
{
	callIF(getFlag(P_FLAG) == true);
}

void CPU8080::CPO(BYTE opcode)
{
	callIF(getFlag(P_FLAG) == false);
}

void CPU8080::retIF(bool condition, int timeT)
{
	if (condition == true)
	{
		BYTE valL, valH;

		m_memory.Read(regSP, valL);
		regSP++;
		m_memory.Read(regSP, valH);
		regSP++;		

		m_timeTicks += timeT;
		
		WORD returnTo = getWord(valH, valL);
		OnReturn(returnTo - 3);
		m_programCounter = returnTo;
	}
	else
	{
		m_timeTicks += 6;
		m_programCounter++;
	}
}

void CPU8080::RET(BYTE opcode)
{
	retIF(true, 10);
}

void CPU8080::RC(BYTE opcode)
{
	retIF(getFlag(CY_FLAG) == true, 12);
}

void CPU8080::RNC(BYTE opcode)
{
	retIF(getFlag(CY_FLAG) == false, 12);
}

void CPU8080::RZ(BYTE opcode)
{
	retIF(getFlag(Z_FLAG) == true, 12);
}

void CPU8080::RNZ(BYTE opcode)
{
	retIF(getFlag(Z_FLAG) == false, 12);
}

void CPU8080::RP(BYTE opcode)
{
	retIF(getFlag(S_FLAG) == false, 12);
}

void CPU8080::RM(BYTE opcode)
{
	retIF(getFlag(S_FLAG) == true, 12);
}

void CPU8080::RPE(BYTE opcode)
{
	retIF(getFlag(P_FLAG) == true, 12);
}

void CPU8080::RPO(BYTE opcode)
{
	retIF(getFlag(P_FLAG) == true, 12);
}

void CPU8080::RST(BYTE opcode)
{
	regSP--;
	m_memory.Write(regSP, getHByte(m_programCounter));
	regSP--;
	m_memory.Write(regSP, getLByte(m_programCounter));

	opcode &= 070;

	m_timeTicks += 12;
	m_programCounter = opcode;
}

void CPU8080::INRr(BYTE opcode)
{
	BYTE &reg = getRegL(opcode);
	reg++;

	adjustParity(reg);
	adjustZero(reg);
	adjustSign(reg);
	setFlag(AC_FLAG, (reg&0x0F)==0);

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::DCRr(BYTE opcode)
{
	BYTE &reg = getRegL(opcode);
	reg--;

	adjustParity(reg);
	adjustZero(reg);
	adjustSign(reg);
	setFlag(AC_FLAG, (reg&0x0F)!=0x0F);

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::INRm(BYTE opcode)
{
	BYTE value;
	m_memory.Read(getHL(), value);
	value++;
	m_memory.Write(getHL(), value);

	adjustParity(value);
	adjustZero(value);
	adjustSign(value);
	setFlag(AC_FLAG, (value&0x0F)==0);

	m_timeTicks += 10;
	m_programCounter++;
}

void CPU8080::DCRm(BYTE opcode)
{
	BYTE value;
	m_memory.Read(getHL(), value);
	value--;
	m_memory.Write(getHL(), value);

	adjustParity(value);
	adjustZero(value);
	adjustSign(value);
	setFlag(AC_FLAG, (value&0x0F)!=0x0F);

	m_timeTicks += 10;
	m_programCounter++;
}

void CPU8080::INXb(BYTE opcode)
{
	WORD value = getWord(regB, regC);
	value++;
	regB = getHByte(value);
	regC = getLByte(value);

	m_timeTicks += 6;
	m_programCounter++;
}

void CPU8080::INXd(BYTE opcode)
{
	WORD value = getWord(regD, regE);
	value++;
	regD = getHByte(value);
	regE = getLByte(value);

	m_timeTicks += 6;
	m_programCounter++;
}

void CPU8080::INXh(BYTE opcode)
{
	WORD value = getHL();
	value++;
	regH = getHByte(value);
	regL = getLByte(value);

	m_timeTicks += 6;
	m_programCounter++;
}

void CPU8080::DCXb(BYTE opcode)
{
	WORD value = getWord(regB, regC);
	value--;
	regB = getHByte(value);
	regC = getLByte(value);

	m_timeTicks += 6;
	m_programCounter++;
}

void CPU8080::DCXd(BYTE opcode)
{
	WORD value = getWord(regD, regE);
	value--;
	regD = getHByte(value);
	regE = getLByte(value);

	m_timeTicks += 6;
	m_programCounter++;
}

void CPU8080::DCXh(BYTE opcode)
{
	WORD value = getHL();
	value--;
	regH = getHByte(value);
	regL = getLByte(value);

	m_timeTicks += 6;
	m_programCounter++;
}

void CPU8080::add(BYTE src, bool carry)
{
	// AC flag
	BYTE loNibble = (regA&0x0F) + (src&0x0F);

	WORD temp = regA + src;
	if (carry)
	{
		temp++;
		loNibble++;
	}

	regA = (BYTE)temp;

	setFlag(CY_FLAG, (temp>0xFF));
	setFlag(AC_FLAG, (loNibble>0x0F));
	adjustSign(regA);
	adjustZero(regA);
	adjustParity(regA);
}

void CPU8080::ADDr(BYTE opcode)
{
	add(getRegR(opcode));

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::ADCr(BYTE opcode)
{
	add(getRegR(opcode), getFlag(CY_FLAG));

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::ADDm(BYTE opcode)
{
	BYTE value;
	m_memory.Read(getHL(), value);

	add(value);

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::ADCm(BYTE opcode)
{
	BYTE value;
	m_memory.Read(getHL(), value);

	add(value, getFlag(CY_FLAG));

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::ADI(BYTE opcode)
{
	BYTE value;
	m_memory.Read(m_programCounter+1, value);

	add(value);

	m_timeTicks += 7;
	m_programCounter+=2;
}

void CPU8080::ACI(BYTE opcode)
{
	BYTE value;
	m_memory.Read(m_programCounter+1, value);

	add(value, getFlag(CY_FLAG));

	m_timeTicks += 7;
	m_programCounter+=2;
}

void CPU8080::dad(WORD value)
{
	long hl = getHL();

	hl += value;

	setFlag(CY_FLAG, hl>0xFFFF);

	regH = getHByte((WORD)hl);
	regL = getLByte((WORD)hl);

	m_timeTicks += 10;
	m_programCounter++;
}

void CPU8080::DADb(BYTE opcode)
{
	dad(getWord(regB, regC));
}

void CPU8080::DADd(BYTE opcode)
{
	dad(getWord(regD, regE));
}

void CPU8080::DADh(BYTE opcode)
{
	dad(getHL());
}

void CPU8080::DADsp(BYTE opcode)
{
	dad(regSP);
}

void CPU8080::sub(BYTE src, bool borrow)
{
	// AC flag
	char loNibble = (regA&0x0F) - (src&0x0F);
	
	int temp = regA - src;
	if (borrow)
	{
		temp--;
		loNibble--;
	}

	regA = (BYTE)temp;

	setFlag(CY_FLAG, (temp<0));
	setFlag(AC_FLAG, !(loNibble<0));
	adjustSign(regA);
	adjustZero(regA);
	adjustParity(regA);

}

void CPU8080::SUBr(BYTE opcode)
{
	sub(getRegR(opcode));

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::SBBr(BYTE opcode)
{
	sub(getRegR(opcode), getFlag(CY_FLAG));

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::SUBm(BYTE opcode)
{
	BYTE value;
	m_memory.Read(getHL(), value);

	sub(value);

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::SBBm(BYTE opcode)
{
	BYTE value;
	m_memory.Read(getHL(), value);

	sub(value, getFlag(CY_FLAG));

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::SUI(BYTE opcode)
{
	BYTE value;
	m_memory.Read(m_programCounter+1, value);

	sub(value);

	m_timeTicks += 7;
	m_programCounter+=2;
}

void CPU8080::SBI(BYTE opcode)
{
	BYTE value;
	m_memory.Read(m_programCounter+1, value);

	sub(value, getFlag(CY_FLAG));

	m_timeTicks += 7;
	m_programCounter+=2;
}

void CPU8080::ANAr(BYTE opcode)
{
	regA &= getRegR(opcode);

	adjustSign(regA);
	adjustZero(regA);
	adjustParity(regA);
	setFlag(AC_FLAG, true);				// Must confirm
	setFlag(CY_FLAG, false);

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::XRAr(BYTE opcode)
{
	regA ^= getRegR(opcode);

	adjustSign(regA);
	adjustZero(regA);
	adjustParity(regA);
	setFlag(AC_FLAG, false);
	setFlag(CY_FLAG, false);

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::ORAr(BYTE opcode)
{
	regA |= getRegR(opcode);

	adjustSign(regA);
	adjustZero(regA);
	adjustParity(regA);
	setFlag(AC_FLAG, false);
	setFlag(CY_FLAG, false);

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::cmp(BYTE src)
{
	// AC flag
	char loNibble = (regA&0x0F) - (src&0x0F);
	
	int temp = regA - src;

	setFlag(CY_FLAG, (temp<0));
	setFlag(AC_FLAG, !(loNibble<0));
	adjustSign((BYTE)temp);
	adjustZero((BYTE)temp);
	adjustParity((BYTE)temp);
}

void CPU8080::CMPr(BYTE opcode)
{
	cmp(getRegR(opcode));

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::ANAm(BYTE opcode)
{
	BYTE value;
	m_memory.Read(getHL(), value);

	regA &= value;

	adjustSign(regA);
	adjustZero(regA);
	adjustParity(regA);
	setFlag(AC_FLAG, true);			// Must confirm
	setFlag(CY_FLAG, false);

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::XRAm(BYTE opcode)
{
	BYTE value;
	m_memory.Read(getHL(), value);

	regA ^= value;

	adjustSign(regA);
	adjustZero(regA);
	adjustParity(regA);
	setFlag(AC_FLAG, false);
	setFlag(CY_FLAG, false);

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::ORAm(BYTE opcode)
{
	BYTE value;
	m_memory.Read(getHL(), value);

	regA |= value;

	adjustSign(regA);
	adjustZero(regA);
	adjustParity(regA);
	setFlag(AC_FLAG, false);
	setFlag(CY_FLAG, false);

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::CMPm(BYTE opcode)
{
	BYTE value;
	m_memory.Read(getHL(), value);

	cmp(value);

	m_timeTicks += 7;
	m_programCounter++;
}

void CPU8080::ANI(BYTE opcode)
{
	BYTE value;
	m_memory.Read(m_programCounter+1, value);

	regA &= value;

	adjustSign(regA);
	adjustZero(regA);
	adjustParity(regA);
	setFlag(AC_FLAG, true);			// Must confirm
	setFlag(CY_FLAG, false);

	m_timeTicks += 7;
	m_programCounter+=2;
}

void CPU8080::XRI(BYTE opcode)
{
	BYTE value;
	m_memory.Read(m_programCounter+1, value);

	regA ^= value;

	adjustSign(regA);
	adjustZero(regA);
	adjustParity(regA);
	setFlag(AC_FLAG, false);
	setFlag(CY_FLAG, false);

	m_timeTicks += 7;
	m_programCounter+=2;
}

void CPU8080::ORI(BYTE opcode)
{
	BYTE value;
	m_memory.Read(m_programCounter+1, value);

	regA |= value;

	adjustSign(regA);
	adjustZero(regA);
	adjustParity(regA);
	setFlag(AC_FLAG, false);
	setFlag(CY_FLAG, false);

	m_timeTicks += 7;
	m_programCounter+=2;
}

void CPU8080::CPI(BYTE opcode)
{
	BYTE value;
	m_memory.Read(m_programCounter+1, value);

	cmp(value);

	m_timeTicks += 7;
	m_programCounter+=2;
}

void CPU8080::RLC(BYTE opcode)
{
	bool msb = (regA & 128)?true:false;

	regA = (regA << 1);
	regA |= (msb?1:0);

	setFlag(CY_FLAG, msb);

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::RRC(BYTE opcode)
{
	bool lsb = (regA & 1)?true:false;

	regA = (regA >> 1);
	regA &= ~128;
	regA |= (lsb?128:0);

	setFlag(CY_FLAG, lsb);

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::RAL(BYTE opcode)
{
	bool msb = (regA & 128)?true:false;

	regA = (regA << 1);
	regA |= (getFlag(CY_FLAG)?1:0);

	setFlag(CY_FLAG, msb);

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::RAR(BYTE opcode)
{
	bool lsb = (regA & 1)?true:false;

	regA = (regA >> 1);
	regA &= ~128;
	regA |= (getFlag(CY_FLAG)?128:0);

	setFlag(CY_FLAG, lsb);

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::CMA(BYTE opcode)
{
	regA = ~regA;

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::STC(BYTE opcode)
{
	setFlag(CY_FLAG, true);

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::CMC(BYTE opcode)
{
	setFlag(CY_FLAG, !getFlag(CY_FLAG));

	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::DAA(BYTE opcode)
{
	Dump();

//	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::IN(BYTE opcode)
{
	BYTE portNb;
	m_memory.Read(m_programCounter+1, portNb);
	m_ports.In(portNb, regA);

	m_timeTicks += 10;
	m_programCounter+=2;
}

void CPU8080::OUT(BYTE opcode)
{
	BYTE portNb;
	m_memory.Read(m_programCounter+1, portNb);
	m_ports.Out(portNb, regA);

	m_timeTicks += 10;
	m_programCounter+=2;
}

void CPU8080::EI(BYTE opcode)
{
	m_interruptsEnabled = true; // TODO: Should happen after next instruction execution
	m_programCounter++;
}

void CPU8080::DI(BYTE opcode)
{
	m_interruptsEnabled = false;
	m_programCounter++;
}

void CPU8080::NOP(BYTE opcode)
{
	m_timeTicks += 4;
	m_programCounter++;
}

void CPU8080::HLT(BYTE opcode)
{
	LogPrintf(LOG_INFO, "HLT\n");
	m_state = STOP;
	m_timeTicks += 5;
	m_programCounter++;
}
