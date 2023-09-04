#include "stdafx.h"

#include <filesystem>
#include "gtest/gtest.h"

using namespace std::filesystem;

#define TEST_DISASSEMBLY 0
#define TEST_CPU 0
#define TEST_FLOPPY 0

const path workingDirectory = "../";

int testDisassembly();
int testCPU();
int testFloppy();

int main(int argc, char* argv[])
{
	current_path(workingDirectory);

#if TEST_DISASSEMBLY
	testDisassembly();
#endif

#if TEST_CPU
	testCPU();
#endif

#if TEST_FLOPPY
	testFloppy();
#endif

	testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();

	return 0;
}
