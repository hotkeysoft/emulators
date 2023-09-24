#include "stdafx.h"

#include "../CPU/CPU8086.h"
#include <functional>

using emul::CPU8086;
using emul::REG16;
using emul::GetLByte;
using emul::GetLWord;
using namespace std::filesystem;
using emul::PortConnector;
using emul::PortConnectorMode;

// JSONTester class runs the processor tests from https://github.com/TomHarte/ProcessorTests/tree/main/8088/v1
// 
// Set path to json test cases here:
const path testCaseFilesPath = "P:/8086/";

const std::string testCaseFileExtension = ".json";
const std::string testCaseSummaryFile = "8088.json";
static constexpr const char* separator = "------------------------------------------";
static constexpr const char* separator2 = "==========================================";

class CPU8086Test : public CPU8086
{
public:
	CPU8086Test(emul::Memory& mem) : CPU8086(mem), Logger("CPU8086Test") {}

	friend class JSONTester;
};

class TesterBase : public Logger
{
public:
	TesterBase(const char* id) : Logger(id), m_memory(1024)
	{
		EnableLog(Logger::LOG_INFO);
		PortConnector::Init(PortConnectorMode::WORD);
		m_memory.Init(emul::CPU8086_ADDRESS_BITS);
		NewCPU();
		LogPrintf(LOG_INFO, "Start");
	}

	virtual ~TesterBase() {};
	virtual void test() = 0;

	void NewCPU()
	{
		delete m_cpu;
		LogPrintf(LOG_INFO, "Creating New 8086 CPU");
		m_cpu = new CPU8086Test(m_memory);
		m_cpu->Init();
	}

	using TestFunc = void (TesterBase::*)();

	void RunTest(const char* id, TestFunc func)
	{
		LogPrintf(LOG_INFO, separator2);
		LogPrintf(LOG_INFO, "START %s", id);
		std::invoke(func, this);
		LogPrintf(LOG_INFO, "END %s", id);
	}

	template <typename T>
	bool ExpectEqual(T expect, T actual, const char* msg = nullptr)
	{
		static_assert(
			std::is_same<T, emul::BYTE>::value ||
			std::is_same<T, emul::WORD>::value ||
			std::is_same<T, emul::DWORD>::value);
		if (expect != actual)
		{
			LogPrintf(LOG_ERROR, "ExpectEqual failed: expect(%X) != actual(%X) [%s]", expect, actual, msg ? msg : "");
			return false;
		}
		return true;
	}

protected:
	emul::Memory m_memory;
	CPU8086Test* m_cpu = nullptr;
};

class JSONTester : public TesterBase
{
public:
	JSONTester() : TesterBase("TEST_JSON"), m_ram("RAM", 1 * 1024 * 1024)
	{
	}

	void setRegisters(const json& state)
	{
		const json& regs = state["regs"];

		m_cpu->m_reg.Get16(REG16::AX) = regs["ax"];
		m_cpu->m_reg.Get16(REG16::BX) = regs["bx"];
		m_cpu->m_reg.Get16(REG16::CX) = regs["cx"];
		m_cpu->m_reg.Get16(REG16::DX) = regs["dx"];
		m_cpu->m_reg.Get16(REG16::CS) = regs["cs"];
		m_cpu->m_reg.Get16(REG16::SS) = regs["ss"];
		m_cpu->m_reg.Get16(REG16::DS) = regs["ds"];
		m_cpu->m_reg.Get16(REG16::ES) = regs["es"];
		m_cpu->m_reg.Get16(REG16::SP) = regs["sp"];
		m_cpu->m_reg.Get16(REG16::BP) = regs["bp"];
		m_cpu->m_reg.Get16(REG16::SI) = regs["si"];
		m_cpu->m_reg.Get16(REG16::DI) = regs["di"];
		m_cpu->m_reg.Get16(REG16::IP) = regs["ip"];

		m_cpu->SetFlags(regs["flags"]);		
	}

	bool validateRegisters(const json& expected)
	{
		const json& regs = expected["regs"];

		bool ret = true;
		ret &= ExpectEqual((emul::WORD)regs["ax"], m_cpu->m_reg.Get16(REG16::AX), "ax");
		ret &= ExpectEqual((emul::WORD)regs["bx"], m_cpu->m_reg.Get16(REG16::BX), "bx");
		ret &= ExpectEqual((emul::WORD)regs["cx"], m_cpu->m_reg.Get16(REG16::CX), "cx");
		ret &= ExpectEqual((emul::WORD)regs["dx"], m_cpu->m_reg.Get16(REG16::DX), "dx");
		ret &= ExpectEqual((emul::WORD)regs["cs"], m_cpu->m_reg.Get16(REG16::CS), "cs");
		ret &= ExpectEqual((emul::WORD)regs["ss"], m_cpu->m_reg.Get16(REG16::SS), "ss");
		ret &= ExpectEqual((emul::WORD)regs["ds"], m_cpu->m_reg.Get16(REG16::DS), "ds");
		ret &= ExpectEqual((emul::WORD)regs["es"], m_cpu->m_reg.Get16(REG16::ES), "es");
		ret &= ExpectEqual((emul::WORD)regs["sp"], m_cpu->m_reg.Get16(REG16::SP), "sp");
		ret &= ExpectEqual((emul::WORD)regs["bp"], m_cpu->m_reg.Get16(REG16::BP), "bp");
		ret &= ExpectEqual((emul::WORD)regs["si"], m_cpu->m_reg.Get16(REG16::SI), "si");
		ret &= ExpectEqual((emul::WORD)regs["di"], m_cpu->m_reg.Get16(REG16::DI), "di");
		ret &= ExpectEqual((emul::WORD)regs["ip"], m_cpu->m_reg.Get16(REG16::IP), "ip");

		emul::WORD expectedFlags = (emul::WORD)regs["flags"] & m_flagMask;
		emul::WORD actualFlags = m_cpu->m_reg.Get16(REG16::FLAGS) & m_flagMask;

		ret &= ExpectEqual(expectedFlags, actualFlags, "flags");

		return ret;
	}

	bool validateRAM(const json& expected)
	{
		bool ret = true;
		for (auto& values : expected["ram"])
		{
			emul::ADDRESS addr = values[0];
			BYTE expected = values[1];

			static char buf[32];
			sprintf(buf, "RAM[%06X]", addr);

			BYTE actual = m_memory.Read8(addr);
			ret &= ExpectEqual(expected, actual, buf);
		}

		return ret;
	}

	void setRAM(const json& state)
	{
		for (auto& values : state["ram"])
		{
			emul::ADDRESS addr = values[0];
			BYTE value = values[1];

			m_memory.Write8(addr, value);
		}
	}

	void testJSON()
	{
		json testCases;

		std::ifstream configFile(m_currPath);
		if (configFile)
		{
			configFile >> testCases;
		}
		else
		{
			LogPrintf(LOG_ERROR, "Error loading test case file: [%s]", m_currPath.string().c_str());
			return;
		}

		if (!testCases.is_array())
		{
			LogPrintf(LOG_ERROR, "Expected array");
			return;
		}

		int tests = (int)testCases.size();
		int fail = 0;
		for (auto& testCase : testCases)
		{
			const json& initial = testCase["initial"];
			const json& final = testCase["final"];

			setRegisters(initial);
			setRAM(initial);
			
			m_cpu->Step();

			bool res = true;
			res &= validateRegisters(final);
			res &= validateRAM(final);
			if (!res)
			{
				std::string name = testCase["name"];
				std::ostringstream bytes;
				bool first = true;
				for (BYTE val : testCase["bytes"])
				{
					if (first) { first = false; } else { bytes << ", "; }
					bytes << (int)val;
				}

				LogPrintf(LOG_ERROR, "... while testing case [%s][%s]", name.c_str(), bytes.str().c_str());
				LogPrintf(LOG_INFO, separator);
				++fail;
			}
		}
		LogPrintf(LOG_INFO, separator);
		LogPrintf(LOG_INFO, "Total tests: %d", tests);
		LogPrintf(LOG_INFO, "Fail: %d", fail);
	}

	virtual void test() override
	{
		m_memory.Allocate(&m_ram, 0);
		for (auto& testCaseFile : m_testCaseFiles)
		{
			m_currPath = testCaseFile;
			m_flagMask = 0xFFFF;

			std::string key = m_currPath.stem().string();

			std::string testName = "Test [" + key + "]";

			// Look for this test in summary file
			if (m_testSummary.contains(key))
			{
				const json& testInfo = m_testSummary[key];
				if (testInfo.contains("status"))
				{
					testName.append("[");
					testName.append(testInfo["status"]);
					testName.append("]");
				}

				if (testInfo.contains("flags-mask"))
				{
					m_flagMask = testInfo["flags-mask"];
				}
			}

			RunTest(testName.c_str(), (TestFunc)(&JSONTester::testJSON));
		}
	}

	bool readTestcaseFiles()
	{
		for (const auto& p : directory_iterator(testCaseFilesPath))
		{
			if (p.path().filename() == testCaseSummaryFile)
			{
				// skip summary file
				continue;
			}

			if (p.path().extension() == testCaseFileExtension)
			{
				m_testCaseFiles.push_back(p);
			}
		}
		LogPrintf(LOG_INFO, "Found %d test case files", m_testCaseFiles.size());
		return true;
	}

	void readSummaryFile()
	{
		path p = testCaseFilesPath;
		p.append(testCaseSummaryFile);

		std::ifstream f(p);
		if (f)
		{
			f  >> m_testSummary;
		}
		else
		{
			LogPrintf(LOG_ERROR, "Error loading summary file: [%s]", p.string().c_str());
			return;
		}

		if (!m_testSummary.is_object())
		{
			LogPrintf(LOG_ERROR, "Test summary file: Expected object");
			return;
		}
	}

	bool setup()
	{
		if (!exists(testCaseFilesPath))
		{			
			LogPrintf(LOG_ERROR, "test case(s) not found: %s\n", testCaseFilesPath.string().c_str());
			return false;
		}

		m_cpu->EnableLog(LOG_WARNING);

		return true;
	}

	emul::MemoryBlock m_ram;
	std::vector<path> m_testCaseFiles;
	json m_testSummary;
	path m_currPath;
	emul::WORD m_flagMask = 0xFFFF;
};


int testCPU()
{

	//{
	//	EAModeTester tester;
	//	tester.test();
	//}

	{
		JSONTester tester;
		if (!tester.setup())
		{
			return 1;
		}
				
		if (!tester.readTestcaseFiles())
		{
			return 2;
		}

		tester.readSummaryFile();
		tester.test();
	}

	return 0;
}
