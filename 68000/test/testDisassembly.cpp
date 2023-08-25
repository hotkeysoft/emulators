#include "stdafx.h"

#include "testDisassembly.h"

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <StringUtil.h>

using namespace std::filesystem;

// Relative to workingDirectory
const path exePath = "../x64/Debug/hotkey68000emu.exe";
const path opcodeFilesPath = "./test/data/";
const std::string opcodeFileExtension = ".tst";
const char* separator = "\n------------------------------------------\n";

size_t totalCount = 0;
size_t totalFailCount = 0;

bool setup()
{
	if (!exists(exePath))
	{
		fprintf(stderr, "exe not found: %s\n", exePath.string().c_str());
		return false;
	}
	if (!exists(opcodeFilesPath))
	{
		fprintf(stderr, "opcode file(s) not found: %s\n", opcodeFilesPath.string().c_str());
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

struct OpcodeFile
{
	std::string name;
	Opcodes opcodes;
	size_t count = 0;
	size_t failCount = 0;
};

using OpcodeFiles = std::vector<OpcodeFile>;
OpcodeFiles opcodeFiles;

OpcodeFile readOpcodeFile(path in)
{
	OpcodeFile ret;
	ret.name = in.filename().string();

	std::ifstream infile(in);
	if (!infile)
	{
		return ret;
	}

	for (std::string line; getline(infile, line); )
	{
		size_t pos = line.find_first_of('\t');
		if (pos == std::string::npos)
		{
			continue;
		}

		Opcode op;
		op.hex = line.substr(0, pos);
		op.disassembly = line.substr(pos + 1);

		ret.opcodes.push_back(op);
	}

	ret.count = ret.opcodes.size();
	ret.failCount = 0;

	fprintf(stdout, "File [%s] => Loaded %zu opcodes to test\n", ret.name.c_str(), ret.count);
	return ret;
}

bool readOpcodeFiles()
{
	for (const auto& p : directory_iterator(opcodeFilesPath))
	{
		if (p.path().extension() == opcodeFileExtension)
		{
			OpcodeFile opcodeFile = readOpcodeFile(p);
			opcodeFiles.push_back(opcodeFile);
			totalCount += opcodeFile.count;
		}
	}
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

bool testOpcode(OpcodeFile& opcodeFile, Opcode& opcode)
{
	try
	{
		std::ostringstream cmd;
		cmd << absolute(exePath) << " -d " << opcode.hex;
		std::string out = exec(cmd.str().c_str());

		if (out != opcode.ToString())
		{
			fprintf(stderr, "\nFAIL!\nexpected: %s\n  actual: %s\n", opcode.ToString().c_str(), out.c_str());
			opcodeFile.failCount += 1;
			++totalFailCount;
		}
	}
	catch (std::exception e)
	{
		fprintf(stderr, "exec failed: %s", e.what());
		return false;
	}

	return true;
}

void ShowResult(std::string name, size_t total, size_t fail)
{
	size_t pass = total - fail;
	fprintf(stdout, "\n[%.40s]\n PASS: %zu\n FAIL: %zu\nTOTAL: %zu\n", name.c_str(), pass, fail, total);
}

bool testOpcodes(OpcodeFile& opcodeFile)
{
	fputs(separator, stdout);
	fputs(opcodeFile.name.c_str(), stdout);

	for (auto& opcode : opcodeFile.opcodes)
	{
		if (!testOpcode(opcodeFile, opcode))
		{
			return false;
		}
	}

	return true;
}

bool testOpcodeFiles()
{
	for (auto& opcodeFile : opcodeFiles)
	{
		if (!testOpcodes(opcodeFile))
		{
			return false;
		}
	}
	return true;
}

int testDisassembly()
{
	if (!setup())
		return 1;
	if (!readOpcodeFiles())
		return 2;
	if (!testOpcodeFiles())
		return 3;

	fputs(separator, stdout);

	for (auto& opcodeFile : opcodeFiles)
	{
		ShowResult(opcodeFile.name, opcodeFile.count, opcodeFile.failCount);
	}

	ShowResult("Total", totalCount, totalFailCount);

	return 0;
}
