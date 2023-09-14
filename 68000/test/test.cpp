#include "stdafx.h"

#include <filesystem>
#include "gtest/gtest.h"
#include <Logger.h>

using namespace std::filesystem;

#define TEST_DISASSEMBLY 0
#define TEST_CPU 1
#define TEST_GTESTS 0

const path workingDirectory = "../";

int testDisassembly();
int testCPU();

void LogCallback(const char* str)
{
	fprintf(stdout, str);
}

extern Logger::SEVERITY TEST_FLOPPY_LOG_LEVEL;

int main(int argc, char* argv[])
{
	current_path(workingDirectory);

	Logger::RegisterLogCallback(LogCallback);

#if TEST_DISASSEMBLY
	testDisassembly();
#endif

#if TEST_CPU
	testCPU();
#endif

#if TEST_GTESTS
	TEST_FLOPPY_LOG_LEVEL = Logger::LOG_INFO;

	testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
#endif

	return 0;
}
