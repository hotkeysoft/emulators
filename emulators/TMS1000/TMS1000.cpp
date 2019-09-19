#include "stdafx.h"
#include "CPUInfo.h"
#include "TMS1000.h"
#include <iostream>

#define SET2(X) (X & 0x03)
#define SET4(X) (X & 0x0F)
#define SET6(X) (X & 0x3F)

namespace TMS1000
{
	Instruction TMS1000Opcodes[256];

	CPUState g_cpu;
	Memory g_memory;

	void nullOp(BYTE) {

	}

	void InitTMS1000()
	{
		// Register to register
		TMS1000Opcodes[0x24] = { nullOp, Format4, "TAY" };
		TMS1000Opcodes[0x23] = { nullOp, Format4, "TYA" };
		TMS1000Opcodes[0x2F] = { nullOp, Format4, "CLA" };

		// Transfer Register to Memory
		TMS1000Opcodes[0x03] = { nullOp, Format4, "TAM" };
		TMS1000Opcodes[0x20] = { nullOp, Format4, "TAMIY" };
		TMS1000Opcodes[0x04] = { nullOp, Format4, "TAMZA" };

		// Memory to Register
		TMS1000Opcodes[0x22] = { nullOp, Format4, "TMY" };
		TMS1000Opcodes[0x21] = { nullOp, Format4, "TMA" };
		TMS1000Opcodes[0x2E] = { nullOp, Format4, "XMA" };

		// Arithmetic
		TMS1000Opcodes[0x25] = { nullOp, Format4, "AMAAC" };
		TMS1000Opcodes[0x27] = { nullOp, Format4, "SAMAN" };
		TMS1000Opcodes[0x28] = { nullOp, Format4, "IMAC" };
		TMS1000Opcodes[0x2A] = { nullOp, Format4, "DMAN" };
		TMS1000Opcodes[0x0E] = { nullOp, Format4, "IA" };
		TMS1000Opcodes[0x2B] = { nullOp, Format4, "IYC" };
		TMS1000Opcodes[0x07] = { nullOp, Format4, "DAN" };
		TMS1000Opcodes[0x2C] = { nullOp, Format4, "DYN" };
		TMS1000Opcodes[0x06] = { nullOp, Format4, "A6AAC" };
		TMS1000Opcodes[0x01] = { nullOp, Format4, "A8AAC" };
		TMS1000Opcodes[0x05] = { nullOp, Format4, "A10AAC" };
		TMS1000Opcodes[0x2D] = { nullOp, Format4, "CPAIZ" };

		// Arithmetic Compare
		TMS1000Opcodes[0x29] = { nullOp, Format4, "ALEM" };

		// Logical Compare
		TMS1000Opcodes[0x26] = { nullOp, Format4, "MNEZ" };
		TMS1000Opcodes[0x02] = { nullOp, Format4, "YNEA" };
		for (BYTE i = 0; i < 16; ++i) {
			// ROM Addressing
			TMS1000Opcodes[0x10 + i] = { nullOp, Format2, "LDP" };
			// Constants
			TMS1000Opcodes[0x40 + i] = { nullOp, Format2, "TCY" };
			// Logical Compare
			TMS1000Opcodes[0x50 + i] = { nullOp, Format2, "YNEC" };
			// Constants
			TMS1000Opcodes[0x60 + i] = { nullOp, Format2, "TCMIY" };
			// Arithmetic Compare
			TMS1000Opcodes[0x70 + i] = { nullOp, Format2, "ALEC" };
		}

		for (BYTE i = 0; i < 4; ++i) {
			// Bits in memory
			TMS1000Opcodes[0x30 + i] = { nullOp, Format3, "SBIT" };
			TMS1000Opcodes[0x34 + i] = { nullOp, Format3, "RBIT" };
			TMS1000Opcodes[0x38 + i] = { nullOp, Format3, "TBIT" };

			// RAM 'X' ADdressing
			TMS1000Opcodes[0x3C + i] = { nullOp, Format3, "LDX" };
		}

		// Input
		TMS1000Opcodes[0x09] = { nullOp, Format4, "KNEZ" };
		TMS1000Opcodes[0x08] = { nullOp, Format4, "TKA" };

		// Output
		TMS1000Opcodes[0x0D] = { nullOp, Format4, "SETR" };
		TMS1000Opcodes[0x0C] = { nullOp, Format4, "RSTR" };
		TMS1000Opcodes[0x0A] = { nullOp, Format4, "TDO" };
		TMS1000Opcodes[0x0B] = { nullOp, Format4, "CLO" };

		// RAM 'X' Addressing
		TMS1000Opcodes[0x00] = { nullOp, Format2, "COMX" };

		// ROM Addressing
		TMS1000Opcodes[0x0F] = { nullOp, Format2, "RETN" };

		for (BYTE i = 0; i < 64; ++i) {
			TMS1000Opcodes[0x80 + i] = { nullOp, Format1, "BR" };
			TMS1000Opcodes[0xC0 + i] = { nullOp, Format1, "CALL" };
		}
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

	void DeleteROM() {
		delete[] g_memory.ROM;
		g_memory.ROM = nullptr;
	}

	void InitROM() {
		DeleteROM();

		g_memory.ROM = new BYTE[g_memory.romSize];
		memset(g_memory.ROM, 0xAA, g_memory.romSize);
	}
	
	void Init(TMS1000Family family, CPUInfo *pCPUInfo) {
		DeleteRAM();
		DeleteROM();

		switch (family) {
		case TMS1000Family::CPU_TMS1000:InitTMS1000();
			break;
		default:
			throw std::exception("Unsupported cpu");
		}

		memset(&g_cpu, 0xAA, sizeof(g_cpu));

		g_memory.ramSize = pCPUInfo->GetRAMWords();
		g_memory.romSize = pCPUInfo->GetROMWords();
		InitRAM();	
		InitROM();
	}

	void Reset() {
		g_cpu.PA = SET4(0xff);
		g_cpu.PC = SET4(0x00);
		g_cpu.S = false;
		g_cpu.SL = false;
		g_cpu.CL = false;
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
			InitROM();
			if (fread(g_memory.ROM, size, 1, f) != 1) {
				throw std::exception("Error reading ROM file");
			}

		}
		fclose(f);
	}

	void Disassemble(BYTE opcode, char* line, int lineSize) {
		memset(line, 'a', lineSize);
		Instruction& instr = TMS1000Opcodes[opcode];
		strcpy_s(line, lineSize, instr.name);
	}
}
