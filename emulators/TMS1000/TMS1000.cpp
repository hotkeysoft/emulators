#include "stdafx.h"
#include "CPUInfo.h"
#include "TMS1000.h"
#include <iostream>

#define SET2(X) (X & 0x03)
#define SET4(X) (X & 0x0F)
#define SET6(X) (X & 0x3F)
#define NOT4(X) ((~X) & 0x0F)

namespace TMS1000
{
	IOCallbackFunc inputCallback = nullptr;
	IOCallbackFunc outputCallback = nullptr;

	long ticks = 0;
	CPUState g_cpu;
	Memory g_memory;

	void ADC(BYTE& reg, BYTE val) {
		BYTE sum = reg + val;
		g_cpu.S = (sum > 0x0F);
		reg = SET4(sum);
	}

	void opTAY(BYTE) {
		// ATN, AUTY
		g_cpu.Y = SET4(g_cpu.A);
		g_cpu.S = true;
	}

	void opTYA(BYTE) {
		// YTP, AUTA
		g_cpu.A = SET4(g_cpu.Y);
		g_cpu.S = true;
	}

	void opCLA(BYTE) {
		// AUTA
		g_cpu.A = 0;
		g_cpu.S = true;
	}

	void opTAM(BYTE) {
		// STO
		PutRAM(g_cpu.A);
		g_cpu.S = true;
	}

	void opTAMIY(BYTE) {
		// STO, YTP, CIN, AUTY
		PutRAM(g_cpu.A);
		g_cpu.Y = SET4(g_cpu.Y + 1);
		g_cpu.S = true;
	}

	void opTAMZA(BYTE) {
		// STO, AUTA
		PutRAM(g_cpu.A);
		g_cpu.A = 0;
		g_cpu.S = true;
	}

	void opTMY(BYTE) {
		// MTP, AUTY
		g_cpu.Y = GetRAM();
		g_cpu.S = true;
	}

	void opTMA(BYTE) {
		// MTP, AUTA
		g_cpu.A = GetRAM();
		g_cpu.S = true;
	}

	void opXMA(BYTE) {
		// MTP, STO, AUTA
		BYTE temp = GetRAM();
		PutRAM(g_cpu.A);
		g_cpu.A = SET4(temp);
		g_cpu.S = true;
	}

	void opAMAAC(BYTE) {
		// MTP, ATN, C8, AUTA
		ADC(g_cpu.A, GetRAM());
	}

	void opSAMAN(BYTE) {
		// MTP, NATN, CIN, C8, AUTA
		BYTE sum = NOT4(g_cpu.A) + GetRAM() + 1;
		g_cpu.S = (sum > 0x0F);
		g_cpu.A = SET4(sum);
	}

	void opIMAC(BYTE) {
		// MTP, CIN, C8, AUTA
		g_cpu.A = GetRAM();
		ADC(g_cpu.A, 0x01);
	}

	void opDMAN(BYTE) {
		// MTP, 15TN, C8, AUTA
		g_cpu.A = GetRAM();
		ADC(g_cpu.A, 0x0F);
	}

	void opIA(BYTE) {
		// ATN, CIN, AUTA
		g_cpu.A = SET4(g_cpu.A+1);
		g_cpu.S = true;
	}

	void opIYC(BYTE) {
		// YTP, CIN, C8, AUTY
		ADC(g_cpu.Y, 0x01);
	}

	void opDAN(BYTE) {
		// CKP, ATN, CIN, C8, AUTA
		ADC(g_cpu.A, 0x0F);
	}

	void opDYN(BYTE) {
		// YTP, 15TN, C8, AUTY
		ADC(g_cpu.Y, 0x0F);
	}

	void opA6AAC(BYTE) {
		// CKP, ATN, C6, AUTA
		ADC(g_cpu.A, 6);
	}

	void opA8AAC(BYTE) {
		// CKP, ATN, C8, AUTA
		ADC(g_cpu.A, 8);
	}
	void opA10AAC(BYTE) {
		// CKP, ATN, C10, AUTA
		ADC(g_cpu.A, 10);
	}

	void opCPAIZ(BYTE) {
		// NATN, CIN, C8, AUTA
		BYTE sum = NOT4(g_cpu.A) + 1;
		g_cpu.S = (sum > 0x0F);
		g_cpu.A = SET4(sum);
	}

	void opALEM(BYTE) {
		// MTP, NATN, CIN, C8
		BYTE sum = NOT4(g_cpu.A) + GetRAM() + 1;
		g_cpu.S = (sum > 0x0F);
	}

	void opALEC(BYTE opcode) {
		// CKP, NATN, CIN, C8
		BYTE sum = NOT4(g_cpu.A) + GetC(opcode) + 1;
		g_cpu.S = (sum > 0x0F);
	}

	void opMNEZ(BYTE) {
		// MTP, NE
		g_cpu.S = (GetRAM() != 0);
	}

	void opYNEA(BYTE) {
		// YTP, ATN, NE, STSL
		g_cpu.S = (g_cpu.A != g_cpu.Y);
		g_cpu.SL = g_cpu.S;
	}

	void opYNEC(BYTE opcode) {
		// YTP, CKN, NE
		g_cpu.S = (g_cpu.Y != GetC(opcode));
	}

	void opSBIT(BYTE opcode) {
		// SBIT
		BYTE setBit = 1 << GetB(opcode);
		PutRAM(GetRAM() | setBit);
		g_cpu.S = true;
	}

	void opRBIT(BYTE opcode) {
		// RBIT
		BYTE setBit = SET4(~(1 << GetB(opcode)));
		PutRAM(GetRAM() & setBit);
		g_cpu.S = true;
	}

	void opTBIT(BYTE opcode) {
		// CKP, CKN, MTP, NE
		g_cpu.S = (GetRAM() & (1 << GetB(opcode)));
	}

	void opTCY(BYTE opcode) {
		// CKP, AUTY
		g_cpu.Y = GetC(opcode);
		g_cpu.S = true;
	}

	void opTCMIY(BYTE opcode) {
		// CKM, YTP, CIN, AUTY
		PutRAM(GetC(opcode));
		g_cpu.Y = SET4(g_cpu.Y + 1);
		g_cpu.S = true;
	}

	void opKNEZ(BYTE) {
		// CKP, NE
		if (inputCallback) {
			inputCallback();
		}
		g_cpu.S = (SET4(g_cpu.K) != 0);
	}

	void opTKA(BYTE) {
		// CKP, AUTA
		if (inputCallback) {
			inputCallback();
		}
		g_cpu.A = SET4(g_cpu.K);
		g_cpu.S = true;
	}

	void opSETR(BYTE) {
		// SETR
		if (g_cpu.Y <= 10) {
			g_cpu.R |= (1 << g_cpu.Y);
		}
		g_cpu.S = true;
		if (outputCallback) {
			outputCallback();
		}
	}

	void opRSTR(BYTE) {
		// RSTR
		if (g_cpu.Y <= 10) {
			g_cpu.R &= (~(1 << g_cpu.Y));
		}
		g_cpu.S = true;
		if (outputCallback) {
			outputCallback();
		}
	}

	void opTDO(BYTE) {
		// TDO
		g_cpu.O = (SET4(g_cpu.A) << 1) | (g_cpu.SL ? 1 : 0);
		g_cpu.S = true;
		if (outputCallback) {
			outputCallback();
		}
	}

	void opCLO(BYTE) {
		// CLO
		g_cpu.O = 0;
		g_cpu.S = true;
		if (outputCallback) {
			outputCallback();
		}
	}

	void opLDX(BYTE opcode) {
		// LDX
		g_cpu.X = GetB(opcode);
		g_cpu.S = true;
	}

	void opCOMX(BYTE) {
		// COMX
		g_cpu.X = SET2(~g_cpu.X);
		g_cpu.S = true;
	}

	void opLDP(BYTE opcode) {
		// LDP
		g_cpu.PB = GetC(opcode);
		g_cpu.S = true;
	}

	void opBR(BYTE opcode) {
		if (g_cpu.S) {
			g_cpu.PC = GetW(opcode);
			
			if (!g_cpu.CL) {
				g_cpu.PA = g_cpu.PB;
			}
		}
		
		g_cpu.S = true;
	}

	void opCALL(BYTE opcode) {
		if (g_cpu.S && !g_cpu.CL) {
			g_cpu.SR = SET6(g_cpu.PC + 1);
			BYTE temp = g_cpu.PB;
			g_cpu.PB = g_cpu.PA;
			g_cpu.PA = temp;
			g_cpu.PC = GetW(opcode);
			g_cpu.CL = true;
		} else if (g_cpu.S && g_cpu.CL) {
			g_cpu.PC = GetW(opcode);
			g_cpu.PB = g_cpu.PA;
		}

		g_cpu.S = true;
	}

	void opRETN(BYTE) {
		if (g_cpu.CL) {
			g_cpu.PC = g_cpu.SR;
			g_cpu.PA = g_cpu.PB;
			g_cpu.CL = false;
		}
		else {
			g_cpu.PA = g_cpu.PB;
		}
		g_cpu.S = true;
	}

	void DeleteRAM() {
		delete[] g_memory.RAM;
		g_memory.RAM = nullptr;
	}

	void InitRAM() {
		DeleteRAM();

		g_memory.RAM = new BYTE[g_memory.ramSize];
		memset(g_memory.RAM, 0xAA, g_memory.ramSize);
	}
	
	void Init(TMS1000Family family, WORD romSize, WORD ramSize) {
		DeleteRAM();

		switch (family) {
		case TMS1000Family::CPU_TMS1000: //InitTMS1000();
			break;
		default:
			throw std::exception("Unsupported cpu");
		}

		g_cpu.X = SET2(0xAA);
		g_cpu.Y = SET4(0xAA);
		g_cpu.A = SET4(0xAA);
		g_cpu.S = false;
		g_cpu.SL = false;
		g_cpu.K = 0;
		g_cpu.O = 0;
		g_cpu.R = 0;
		g_cpu.PA = SET4(0xFF);
		g_cpu.PB = SET4(0xFF);
		g_cpu.PC = SET6(0x0);
		g_cpu.SR = SET6(0x0);
		g_cpu.CL = false;

		g_memory.ramSize = ramSize;
		g_memory.romSize = romSize;
		InitRAM();	
	}

	void Reset() {
		g_cpu.PA = SET4(0xff);
		g_cpu.PB = SET4(0xff);
		g_cpu.PC = SET6(0x00);
		g_cpu.S = false;
		g_cpu.SL = false;
		g_cpu.CL = false;
		ticks = 0;
	}

	BYTE GetW(BYTE opcode) {
		return opcode & 0x3F;
	}
	BYTE GetF(BYTE opcode) {
		return	(opcode & 1 ? 4 : 0) |
				(opcode & 2 ? 2 : 0) |
				(opcode & 4 ? 1 : 0);
	}
	BYTE GetC(BYTE opcode) {
		return	(opcode & 1 ? 8 : 0) | 
				(opcode & 2 ? 4 : 0) |
				(opcode & 4 ? 2 : 0) |
				(opcode & 8 ? 1 : 0);
	}
	BYTE GetB(BYTE opcode) {
		return 
			(opcode & 1 ? 2 : 0) | 
			(opcode & 2 ? 1 : 0);
	}

	BYTE GetM() {
		return GetM(g_cpu.X, g_cpu.Y);
	}
	BYTE GetM(BYTE x, BYTE y) {
		return ((x & 0x03) << 4) + (y & 0x0F);
	}
	BYTE GetRAM(BYTE addr) {
		return g_memory.RAM[addr] & 0x0F;
	}
	void PutRAM(BYTE value, BYTE addr) {
		g_memory.RAM[addr] = (value & 0x0F);
	}

	BYTE GetROM() {
		WORD baseAddr = (SET4(TMS1000::g_cpu.PA) * 64) + SET6(g_cpu.PC);
		return g_memory.ROM[baseAddr];
	}

	void Exec(BYTE opcode) {
//		TMS1000Opcodes[opcode].func(opcode);
	}

	void Step() {	
		BYTE oldPC = g_cpu.PC;
		BYTE opCode = GetROM();
		Exec(opCode);
		if (g_cpu.PC == oldPC) {
			g_cpu.PC = SET6(g_cpu.PC + 1);
		}
		ticks += 6;
	}

	void SetInputCallback(IOCallbackFunc func) {
		inputCallback = func;
	}

	void SetOutputCallback(IOCallbackFunc func) {
		outputCallback = func;
	}

	long GetTicks() {
		return ticks;
	}

#ifndef ARDUINO
	// The program counter is not linear, it increments in 
	// this sequence
	static BYTE PCSequence[64] = {
		0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x3E,
		0x3D, 0x3B, 0x37, 0x2F, 0x1E, 0x3C, 0x39, 0x33,
		0x27, 0x0E, 0x1D, 0x3A, 0x35, 0x2B, 0x16, 0x2C,
		0x18, 0x30, 0x21, 0x02, 0x05, 0x0B, 0x17, 0x2E,
		0x1C, 0x38, 0x31, 0x23, 0x06, 0x0D, 0x1B, 0x36,
		0x2D, 0x1A, 0x34, 0x29, 0x12, 0x24, 0x08, 0x11,
		0x22, 0x04, 0x09, 0x13, 0x26, 0x0C, 0x19, 0x32,
		0x25, 0x0A, 0x15, 0x2A, 0x14, 0x28, 0x10, 0x20
	};

	BYTE inverseSequence(BYTE addr) {
		for (BYTE i = 0; i < 64; ++i) {
			if (PCSequence[i] == addr) {
				return i;
			}
		}
		throw std::exception("can't happen");
	}

	// Rearrange the ROM according to the PC sequence
	// so it appear as linear (and then we can simply 
	// increment PC)
	void RemapROM() {
		BYTE *remappedROM = new BYTE[g_memory.romSize];

		for (int i = 0; i < g_memory.romSize; ++i) {
			// Remap memory addresses
			remappedROM[i] = g_memory.ROM[i & 0xFFC0 | PCSequence[i & 0x3F]];

			// Remap branch and jump instructions
			if (remappedROM[i] & 0x80) {
				BYTE newAddress = inverseSequence(remappedROM[i] & 0x3F);
				remappedROM[i] &= 0xC0; // Keep opcode
				remappedROM[i] |= newAddress; // new operand
			}
		}

		delete g_memory.ROM;
		g_memory.ROM = remappedROM;
	}

	void LoadROM(const char* path) {
		FILE* f = fopen(path, "rb");
		if (f) {
			fseek(f, 0, SEEK_END);
			int size = ftell(f);
			if (g_memory.romSize != size) {
				throw std::exception("ROM size mismatch");
			}
			fseek(f, 0, SEEK_SET);
			delete[] g_memory.ROM;
			g_memory.ROM = new BYTE[g_memory.romSize];
			if (fread(g_memory.ROM, size, 1, f) != 1) {
				throw std::exception("Error reading ROM file");
			}
			RemapROM();
		}
		fclose(f);
	}

	void SaveROM(const char* path) {
		FILE* f = fopen(path, "w");
		if (f) {
			fprintf(f, "const BYTE rom[] PROGMEM = {\n\t");
			int lines = g_memory.romSize / 16;
			for (int y = 0; y < lines; ++y) {
				for (int x = 0; x < 16; ++x) {
					fprintf(f, "0x%02X, ", g_memory.ROM[y * 16 + x]);
				}
				fprintf(f, "\n\t");
			}
			fprintf(f, "};\n");
		}
		fclose(f);
	}
#endif

}
