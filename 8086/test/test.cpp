#include "stdafx.h"

#include <filesystem>
#include <Logger.h>

using namespace std::filesystem;

#define TEST_CPU 1

const path workingDirectory = "../";

int testCPU();

void LogCallback(const char* str)
{
	fprintf(stdout, str);
}

int main(int argc, char* argv[])
{
	current_path(workingDirectory);

	Logger::RegisterLogCallback(LogCallback);

#if TEST_CPU
	testCPU();
#endif

	return 0;
}
