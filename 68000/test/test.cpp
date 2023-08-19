#include "pch.h"
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <StringUtil.h>

using namespace std::filesystem;

const path workingDirectory = "../";

// Relative to workingDirectory
const path exePath = "../x64/Debug/hotkey68000emu.exe";
const path opcodeFile = "./test/data/opcodes.txt";

bool setup()
{
	current_path(workingDirectory);

	if (!exists(exePath))
	{
		fprintf(stderr, "exe not found: %s\n", exePath.string().c_str());
		return false;
	}
	if (!exists(opcodeFile))
	{
		fprintf(stderr, "opcode file not found: %s\n", opcodeFile.string().c_str());
		return false;
	}

	return true;
}

struct Opcode
{
	std::string hex;
	std::string disassembly;

	std::string ToString() const {
		std::ostringstream os;
		os << hex << '\t' << disassembly;
		return os.str();
	}
};

using Opcodes = std::vector<Opcode>;
Opcodes opcodes;

bool readOpcodeFile()
{
	std::ifstream infile(opcodeFile);
	if (!infile)
	{
		return false;
	}

	for (std::string line; getline(infile, line); )
	{
		size_t pos = line.find_first_of('\t');
		if (pos == std::string::npos)
		{
			return false;
		}

		Opcode op;
		op.hex = line.substr(0, pos);
		op.disassembly = line.substr(pos + 1);

		opcodes.push_back(op);
	}

	fprintf(stdout, "Loaded %zu opcodes to test", opcodes.size());

	return true;
}

#ifdef _WINDOWS
FILE* popen(const char* command, const char* flags) { return _popen(command, flags); }
int pclose(FILE* fd) { return _pclose(fd); }
#endif

std::string exec(const char* cmd)
{
	std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
	if (!pipe)
	{
		throw std::exception("popen failed");
	}

	char buf[128];
	std::string result;
	while (!feof(pipe.get()))
	{
		if (fgets(buf, 128, pipe.get()) != NULL)
		{
			result += buf;
		}
	}
	StringUtil::Trim(result);
	return result;
}

size_t failCount = 0;

bool testOpcode(Opcode& opcode)
{
	try
	{
		std::ostringstream cmd;
		cmd << absolute(exePath) << " -d " << opcode.hex;
		std::string out = exec(cmd.str().c_str());

		if (out != opcode.ToString())
		{
			fprintf(stderr, "\nFAIL!\nexpected: %s\n  actual: %s\n", opcode.ToString().c_str(), out.c_str());
			++failCount;
		}
	}
	catch (std::exception e)
	{
		fprintf(stderr, "exec failed: %s", e.what());
		return false;
	}

	return true;
}

bool testOpcodes()
{
	for (auto& opcode : opcodes)
	{
		if (!testOpcode(opcode))
		{
			return false;
		}
	}
	return true;
}

int main()
{
	if (!setup())
		return 1;
	if (!readOpcodeFile())
		return 2;
	if (!testOpcodes())
		return 3;

	size_t total = opcodes.size();
	size_t pass = total - failCount;
	fprintf(stdout, "\nDone!\nPASS: %zu\nFAIL: %zu\nTOTAL: %zu\n", pass, failCount, total);

	return 0;
}
