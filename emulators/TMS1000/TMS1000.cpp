#ifdef ARDUINO 
	#include <avr/pgmspace.h>
#else
	#include <iostream>
#endif
#include "TMS1000.h"

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

	void uADC(BYTE& reg, BYTE val) {
		BYTE sum = reg + val;
		g_cpu.S = (sum > 0x0F);
		reg = SET4(sum);
	}

	void opTAY() {
		// ATN, AUTY
		g_cpu.Y = SET4(g_cpu.A);
	}

	void opTYA() {
		// YTP, AUTA
		g_cpu.A = SET4(g_cpu.Y);
	}

	void opCLA() {
		// AUTA
		g_cpu.A = 0;
	}

	void opTAM() {
		// STO
		PutRAM(g_cpu.A);
	}

	void opTAMIY() {
		// STO, YTP, CIN, AUTY
		PutRAM(g_cpu.A);
		g_cpu.Y = SET4(g_cpu.Y + 1);
	}

	void opTAMZA() {
		// STO, AUTA
		PutRAM(g_cpu.A);
		g_cpu.A = 0;
	}

	void opTMY() {
		// MTP, AUTY
		g_cpu.Y = GetRAM();
	}

	void opTMA() {
		// MTP, AUTA
		g_cpu.A = GetRAM();
	}

	void opXMA() {
		// MTP, STO, AUTA
		BYTE temp = GetRAM();
		PutRAM(g_cpu.A);
		g_cpu.A = SET4(temp);
	}

	void opAMAAC() {
		// MTP, ATN, C8, AUTA
		uADC(g_cpu.A, GetRAM());
	}

	void opSAMAN() {
		// MTP, NATN, CIN, C8, AUTA
		BYTE sum = NOT4(g_cpu.A) + GetRAM() + 1;
		g_cpu.S = (sum > 0x0F);
		g_cpu.A = SET4(sum);
	}

	void opIMAC() {
		// MTP, CIN, C8, AUTA
		g_cpu.A = GetRAM();
		uADC(g_cpu.A, 0x01);
	}

	void opDMAN() {
		// MTP, 15TN, C8, AUTA
		g_cpu.A = GetRAM();
		uADC(g_cpu.A, 0x0F);
	}

	void opIA() {
		// ATN, CIN, AUTA
		g_cpu.A = SET4(g_cpu.A+1);
	}

	void opIYC() {
		// YTP, CIN, C8, AUTY
		uADC(g_cpu.Y, 0x01);
	}

	void opDAN() {
		// CKP, ATN, CIN, C8, AUTA
		uADC(g_cpu.A, 0x0F);
	}

	void opDYN() {
		// YTP, 15TN, C8, AUTY
		uADC(g_cpu.Y, 0x0F);
	}

	void opA6AAC() {
		// CKP, ATN, C6, AUTA
		uADC(g_cpu.A, 6);
	}

	void opA8AAC() {
		// CKP, ATN, C8, AUTA
		uADC(g_cpu.A, 8);
	}
	void opA10AAC() {
		// CKP, ATN, C10, AUTA
		uADC(g_cpu.A, 10);
	}

	void opCPAIZ() {
		// NATN, CIN, C8, AUTA
		BYTE sum = NOT4(g_cpu.A) + 1;
		g_cpu.S = (sum > 0x0F);
		g_cpu.A = SET4(sum);
	}

	void opALEM() {
		// MTP, NATN, CIN, C8
		BYTE sum = NOT4(g_cpu.A) + GetRAM() + 1;
		g_cpu.S = (sum > 0x0F);
	}

	void opALEC(BYTE value) {
		// CKP, NATN, CIN, C8
		BYTE sum = NOT4(g_cpu.A) + value + 1;
		g_cpu.S = (sum > 0x0F);
	}

	void opMNEZ() {
		// MTP, NE
		g_cpu.S = (GetRAM() != 0);
	}

	void opYNEA() {
		// YTP, ATN, NE, STSL
		g_cpu.S = (g_cpu.A != g_cpu.Y);
		g_cpu.SL = g_cpu.S;
	}

	void opYNEC(BYTE value) {
		// YTP, CKN, NE
		g_cpu.S = (g_cpu.Y != value);
	}

	void opSBIT(BYTE value) {
		// SBIT
		BYTE setBit = 1 << value;
		PutRAM(GetRAM() | setBit);
	}

	void opRBIT(BYTE value) {
		// RBIT
		BYTE setBit = SET4(~(1 << value));
		PutRAM(GetRAM() & setBit);
	}

	void opTBIT1(BYTE value) {
		// CKP, CKN, MTP, NE
		g_cpu.S = (GetRAM() & (1 << value));
	}

	void opTCY(BYTE value) {
		// CKP, AUTY
		g_cpu.Y = value;
	}

	void opTCMIY(BYTE value) {
		// CKM, YTP, CIN, AUTY
		PutRAM(value);
		g_cpu.Y = SET4(g_cpu.Y + 1);
	}

	void opKNEZ() {
		// CKP, NE
		if (inputCallback) {
			inputCallback();
		}
		g_cpu.S = (SET4(g_cpu.K) != 0);
	}

	void opTKA() {
		// CKP, AUTA
		if (inputCallback) {
			inputCallback();
		}
		g_cpu.A = SET4(g_cpu.K);
	}

	void opSETR() {
		// SETR
		if (g_cpu.Y <= 10) {
			g_cpu.R |= (1 << g_cpu.Y);
		}
		if (outputCallback) {
			outputCallback();
		}
	}

	void opRSTR() {
		// RSTR
		if (g_cpu.Y <= 10) {
			g_cpu.R &= (~(1 << g_cpu.Y));
		}
		if (outputCallback) {
			outputCallback();
		}
	}

	void opTDO() {
		// TDO
		g_cpu.O = (SET4(g_cpu.A) << 1) | (g_cpu.SL ? 1 : 0);
		if (outputCallback) {
			outputCallback();
		}
	}

	void opCLO() {
		// CLO
		g_cpu.O = 0;
		if (outputCallback) {
			outputCallback();
		}
	}

	void opLDX(BYTE value) {
		// LDX
		g_cpu.X = value;
	}

	void opCOMX() {
		// COMX
		g_cpu.X = SET2(~g_cpu.X);
	}

	void opLDP(BYTE value) {
		// LDP
		g_cpu.PB = value;
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

	void opRETN() {
		g_cpu.PA = g_cpu.PB;
		if (g_cpu.CL) {
			g_cpu.PC = g_cpu.SR;
			g_cpu.CL = false;
		}
	}

	void DeleteRAM() {
		delete[] g_memory.RAM;
		g_memory.RAM = nullptr;
	}

	void InitRAM() {
		DeleteRAM();

		g_memory.RAM = new BYTE[g_memory.ramSize];
	}
	
	void Init(WORD romSize, WORD ramSize) {
		DeleteRAM();

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
		return SET6(opcode);
	}

	// GetB, GetC, and GetF are helpers and not used internally because they are a bit costly
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
		return (SET2(x) << 4) | SET4(y);
	}
	BYTE GetRAM(BYTE addr) {
		return SET4(g_memory.RAM[addr]);
	}
	void PutRAM(BYTE value, BYTE addr) {
		g_memory.RAM[addr] = SET4(value);
	}

	BYTE GetROM() {
		WORD baseAddr = (SET4(TMS1000::g_cpu.PA) << 6) + SET6(g_cpu.PC);
#ifdef ARDUINO
		return pgm_read_byte_near(g_memory.ROM + baseAddr);
#else
		return g_memory.ROM[baseAddr];
#endif
	}

	void Exec(BYTE opcode) {
		if (!(opcode & 0x80)) {
			g_cpu.S = true;
		}
		switch (opcode) {
		// Register to register
		case 0x24: opTAY(); break;
		case 0x23: opTYA(); break;
		case 0x2F: opCLA(); break;

		// Transfer Register to Memory
		case 0x03: opTAM(); break;
		case 0x20: opTAMIY(); break;
		case 0x04: opTAMZA(); break;

		// Memory to Register
		case 0x22: opTMY(); break;
		case 0x21: opTMA(); break;
		case 0x2E: opXMA(); break;

		// Arithmetic
		case 0x25: opAMAAC(); break;
		case 0x27: opSAMAN(); break;
		case 0x28: opIMAC(); break;
		case 0x2A: opDMAN(); break;
		case 0x0E: opIA(); break;
		case 0x2B: opIYC(); break;
		case 0x07: opDAN(); break;
		case 0x2C: opDYN(); break;
		case 0x06: opA6AAC(); break;
		case 0x01: opA8AAC(); break;
		case 0x05: opA10AAC(); break;
		case 0x2D: opCPAIZ(); break;

		// Arithmetic Compare
		case 0x29: opALEM(); break;

		case 0x70: opALEC(0);  break;
		case 0x71: opALEC(8);  break;
		case 0x72: opALEC(4);  break;
		case 0x73: opALEC(12); break;
		case 0x74: opALEC(2);  break;
		case 0x75: opALEC(10); break;
		case 0x76: opALEC(6);  break;
		case 0x77: opALEC(14); break;
		case 0x78: opALEC(1);  break;
		case 0x79: opALEC(9);  break;
		case 0x7A: opALEC(5);  break;
		case 0x7B: opALEC(13); break;
		case 0x7C: opALEC(3);  break;
		case 0x7D: opALEC(11); break;
		case 0x7E: opALEC(7);  break;
		case 0x7F: opALEC(15); break;

		// Logical Compare
		case 0x26: opMNEZ(); break;
		case 0x02: opYNEA(); break;

		case 0x50: opYNEC(0);  break;
		case 0x51: opYNEC(8);  break;
		case 0x52: opYNEC(4);  break;
		case 0x53: opYNEC(12); break;
		case 0x54: opYNEC(2);  break;
		case 0x55: opYNEC(10); break;
		case 0x56: opYNEC(6);  break;
		case 0x57: opYNEC(14); break;
		case 0x58: opYNEC(1);  break;
		case 0x59: opYNEC(9);  break;
		case 0x5A: opYNEC(5);  break;
		case 0x5B: opYNEC(13); break;
		case 0x5C: opYNEC(3);  break;
		case 0x5D: opYNEC(11); break;
		case 0x5E: opYNEC(7);  break;
		case 0x5F: opYNEC(15); break;

		// Bits in memory
		case 0x30: opSBIT(0); break;
		case 0x31: opSBIT(2); break;
		case 0x32: opSBIT(1); break;
		case 0x33: opSBIT(3); break;

		case 0x34: opRBIT(0); break;
		case 0x35: opRBIT(2); break;
		case 0x36: opRBIT(1); break;
		case 0x37: opRBIT(3); break;

		case 0x38: opTBIT1(0); break;
		case 0x39: opTBIT1(2); break;
		case 0x3A: opTBIT1(1); break;
		case 0x3B: opTBIT1(3); break;

		// Constants
		case 0x40: opTCY(0);  break;
		case 0x41: opTCY(8);  break;
		case 0x42: opTCY(4);  break;
		case 0x43: opTCY(12); break;
		case 0x44: opTCY(2);  break;
		case 0x45: opTCY(10); break;
		case 0x46: opTCY(6);  break;
		case 0x47: opTCY(14); break;
		case 0x48: opTCY(1);  break;
		case 0x49: opTCY(9);  break;
		case 0x4A: opTCY(5);  break;
		case 0x4B: opTCY(13); break;
		case 0x4C: opTCY(3);  break;
		case 0x4D: opTCY(11); break;
		case 0x4E: opTCY(7);  break;
		case 0x4F: opTCY(15); break;

		case 0x60: opTCMIY(0);  break;
		case 0x61: opTCMIY(8);  break;
		case 0x62: opTCMIY(4);  break;
		case 0x63: opTCMIY(12); break;
		case 0x64: opTCMIY(2);  break;
		case 0x65: opTCMIY(10); break;
		case 0x66: opTCMIY(6);  break;
		case 0x67: opTCMIY(14); break;
		case 0x68: opTCMIY(1);  break;
		case 0x69: opTCMIY(9);  break;
		case 0x6A: opTCMIY(5);  break;
		case 0x6B: opTCMIY(13); break;
		case 0x6C: opTCMIY(3);  break;
		case 0x6D: opTCMIY(11); break;
		case 0x6E: opTCMIY(7);  break;
		case 0x6F: opTCMIY(15); break;

		// Input
		case 0x09: opKNEZ(); break;
		case 0x08: opTKA(); break;

		// Output
		case 0x0D: opSETR(); break;
		case 0x0C: opRSTR(); break;
		case 0x0A: opTDO(); break;
		case 0x0B: opCLO(); break;

		// RAM 'X' Addressing
		case 0x3C: opLDX(0); break;
		case 0x3D: opLDX(2); break;
		case 0x3E: opLDX(1); break;
		case 0x3F: opLDX(3); break;

		case 0x00: opCOMX(); break;

		// ROM Addressing
		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F:
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F:
		case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7:
		case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF:
		case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
		case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF:
			opBR(opcode); break;

		case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7:
		case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF:
		case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7:
		case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDE: case 0xDF:
		case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
		case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
		case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF7:
		case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF:
			opCALL(opcode); break;

		// ROM Addressing
		case 0x0F: opRETN(); break;

		case 0x10: opLDP(0);  break;
		case 0x11: opLDP(8);  break;
		case 0x12: opLDP(4);  break;
		case 0x13: opLDP(12); break;
		case 0x14: opLDP(2);  break;
		case 0x15: opLDP(10); break;
		case 0x16: opLDP(6);  break;
		case 0x17: opLDP(14); break;
		case 0x18: opLDP(1);  break;
		case 0x19: opLDP(9);  break;
		case 0x1A: opLDP(5);  break;
		case 0x1B: opLDP(13); break;
		case 0x1C: opLDP(3);  break;
		case 0x1D: opLDP(11); break;
		case 0x1E: opLDP(7);  break;
		case 0x1F: opLDP(15); break;
		}
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
			if (fread(const_cast<TMS1000::BYTE*>(g_memory.ROM), size, 1, f) != 1) {
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