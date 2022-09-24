// Emulator Base.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <CPU/Memory.h>
#include <CPU/MemoryBlock.h>
#include "CPU8080.h"
#include "UART.h"
#include <vector>

#include <Windows.h>

class Timer : public emul::InterruptSource
{
public:
	Timer() : Logger("TIMER") {}

	virtual bool IsInterrupting() override
	{
		DWORD ticks = GetTickCount();
		if (m_start == 0)
		{
			m_start = ticks;
			return false;
		}

		DWORD delta = ticks - m_start;
		if (delta > 100)
		{
			m_start = ticks;
			return true;
		}
		return false;
	}

	DWORD m_start = 0;
};

void LogCallback(const char *str)
{
	fprintf(stderr, str);
}

const char HexNumbers[] = "0123456789ABCDEF";

BYTE hexToByte(const std::string &hexStr)
{
	if (hexStr.length() < 2)
		return 0;

	const char* n1 = strchr(HexNumbers, toupper(hexStr[0]));
	const char* n2 = strchr(HexNumbers, toupper(hexStr[1]));

	if (!n1 || !n2)
		throw std::exception("Invalid hex digit");

	return BYTE(n1-HexNumbers)*16 + BYTE(n2-HexNumbers);
}

WORD hexToWord(const std::string &hexStr)
{
	if (hexStr.length() < 4)
		return 0;

	return hexToByte(hexStr.substr(0, 2)) * 256 +
		hexToByte(hexStr.substr(2, 2));
}

void SaveROMBlock(emul::MemoryBlock& out, emul::ADDRESS addr, std::vector<BYTE>& buffer)
{
	fprintf(stdout, "Saving block 0x%04X-0x%04X, size: %d\n", addr, addr + (int)buffer.size() - 1, (int)buffer.size());
	out.Fill(addr, buffer);
}

// ASLINK map file
//bool readMapFile(const std::string &fileName, emul::MemoryMap &mmap)
//{
//	std::ifstream file(fileName.c_str(), std::ios::in);
//
//	if (!file)
//	{
//		std::cerr << "Error opening file: " << fileName << std::endl;
//		return false;
//	}
//
//	int lastAddr = -1;
//	int items = 0;
//	std::vector<BYTE> currBuffer;
//
//	while (file)
//	{
//		std::string line;
//		std::getline(file, line);
//
//		if (!file)
//			break;
//
//		// Lines with map info start with 10 spaces ("          ")
//		size_t pos = line.find_first_not_of(' ');
//		if (pos != 10)
//		{
//			continue;
//		}
//
//		std::istringstream is(line);
//		emul::MemoryMapItem item;
//		is >> std::hex >> item.address >> item.label >> item.module;
//
//		mmap.Add(item);
//		++items;
//	}
//	std::cout << "MemoryMap: Added " << items << " labels" << std::endl;
//
//	return true;
//}

// TODO: For now we only support filling-in a preallocated block starting at address 0
bool readIntelHex(const std::string &fileName, emul::MemoryBlock &data)
{
	std::ifstream file(fileName.c_str(), std::ios::in);

	if (!file)
	{
		std::cerr << "Error opening file: " << fileName << std::endl;
		return false;
	}

	int lastAddr = -1;
	std::vector<BYTE> currBuffer;

	while (file)
	{
		std::string line;
		std::getline(file, line);

		if (!file)
			break;

		if (line[0] != ':')	// Lines should begin with S
			goto abort;

		BYTE nbBytes = hexToByte(line.substr(1, 2));

		if (line.length() - 11 < nbBytes * 2)
		{
			std::cerr << "Unexpected record length" << std::endl;
			goto abort;
		}

		WORD blockAddr = hexToWord(line.substr(3, 4));
		BYTE type = hexToByte(line.substr(7, 1));
		BYTE checksum = hexToByte(line.substr((nbBytes*2)+9, 2));

		if (type == 1) // record type = eof
		{
			if (lastAddr != -1) // save previous block, if any
			{
				SaveROMBlock(data, lastAddr, currBuffer);
			}
			break;
		}

		if (type != 0) // record type should be 'data'
		{
			std::cerr << "Unexpected record type" << std::endl;
			goto abort;
		}

		if (lastAddr + currBuffer.size() != blockAddr)	// new block
		{
			if (lastAddr != -1) // save previous block
			{
				SaveROMBlock(data, lastAddr, currBuffer);
			}
			lastAddr = blockAddr;
			currBuffer.clear();
		}

		BYTE sum = 0;
		for (int i = 0; i<nbBytes + 4; i++)
		{
			BYTE currByte = hexToByte(line.substr(i * 2 + 1, 2));
			sum += currByte;

			if (i >= 4) {
				currBuffer.push_back(currByte);
			}
		}

		sum += checksum;
		if (sum != 0)
		{
			std::cerr << "Checksum error" << std::endl;
			goto abort;
		}
	}

	file.close();
	return true;

abort:
	file.close();
	std::cerr << "Abort." << std::endl;
	return false;
}

bool readSRecord(const std::string &fileName, emul::MemoryBlock& data)
{
	std::ifstream file(fileName.c_str(), std::ios::in);

	if (!file)
	{
		std::cerr << "Error opening file: " << fileName << std::endl;
		return false;
	}

	int lastAddr = -1;
	std::vector<BYTE> currBuffer;

	while (file)
	{
		std::string temp;
		std::getline(file, temp);

		if (!file)
			break;

		if (temp[0] != 'S')	// Lines should begin with S
			goto abort;

		if (temp[1] == '9') // record type = eof
		{
			if (lastAddr != -1) // save previous block, if any
			{
				SaveROMBlock(data, lastAddr, currBuffer);
			}
			break;
		}

		if (temp[1] != '1') // record type should be 'data'
			goto abort;

		BYTE nbBytes = hexToByte(temp.substr(2,2));

		if (temp.length()-4 < nbBytes*2)
			goto abort;

		nbBytes -= 3;

		WORD blockAddr = hexToWord(temp.substr(4, 4));

		if (lastAddr+currBuffer.size() != blockAddr)	// new block
		{
			if (lastAddr != -1) // save previous block
			{
				SaveROMBlock(data, lastAddr, currBuffer);
			}
			lastAddr = blockAddr;
			currBuffer.clear();
		}

		for (int i=0; i<nbBytes; i++)
		{
			currBuffer.push_back(hexToByte(temp.substr(i*2+8,2)));
		}
	}

	file.close();
	return true;

abort:
	file.close();
	std::cerr << "Error reading data" << std::endl;
	return false;
}

unsigned long elapsed;

void onCall(emul::CPU* cpu, emul::WORD addr)
{
	elapsed = cpu->getTime();
}

void onRet(emul::CPU* cpu, emul::WORD addr)
{
	fprintf(stderr, "\tELAPSED: %ul\n", cpu->getTime()-elapsed);
}

int main(void)
{
	Logger::RegisterLogCallback(LogCallback);

	emul::Memory memory(1024);
	memory.Init(16);
	memory.EnableLog(Logger::LOG_INFO);

	emul::MemoryBlock rom("ROM", 0x8000, emul::MemoryType::ROM);
	if (readIntelHex("data/main.ihx", rom))
	{
		memory.Allocate(&rom, 0);
	}

	emul::MemoryBlock buffer_memory("RAM", 0x8000);
	memory.Allocate(&buffer_memory, 0x8000);

	Timer timer;

	emul::UART uart(0x60);
	uart.Init();
	uart.EnableLog(Logger::LOG_OFF);

	emul::Interrupts interrupts;
	interrupts.Allocate(emul::CPU8080::RST65, &uart);
	interrupts.Allocate(emul::CPU8080::TRAP, &timer);

	emul::CPU8080 cpu(memory, interrupts);
	//cpu.GetMemoryMap().EnableLog(Logger::LOG_OFF);

	//if (!readMapFile("data/main.map", cpu.GetMemoryMap()))
	//{
	//	fprintf(stderr, "Error reading map file\n");
	//}

	//cpu.AddWatch("EXECUTE", onCall, onRet);

	cpu.AddDevice(uart);
	cpu.Reset();

	fprintf(stderr, "Press any key to continue\n");
	_getch();

	time_t startTime, stopTime;
	time(&startTime);

	while (cpu.Step() && !uart.IsEscape()) {};

	time(&stopTime);

	fprintf(stderr, "Time elapsed: %I64u\n", stopTime-startTime);
	cpu.getTime();
	fprintf(stderr, "CPU ticks: %u\n", cpu.getTime());
	if (stopTime - startTime > 1)
	{
		fprintf(stderr, "Avg speed: %I64u ticks/s\n", cpu.getTime() / (stopTime - startTime));
	}

	return 0;
}


