#include "stdafx.h"

#include <filesystem>

using namespace std::filesystem;

#include "testDisassembly.h"
#include "testCPU.h"

//#define TEST_DISASSEMBLY
#define TEST_CPU

const path workingDirectory = "../";


int main()
{
	current_path(workingDirectory);

#ifdef TEST_DISASSEMBLY
	testDisassembly();
#endif

#ifdef TEST_CPU
	testCPU();
#endif

	return 0;
}
