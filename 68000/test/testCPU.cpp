#include "stdafx.h"
#include "testCPU.h"

#include "../CPU/CPU68000.h"
#include <functional>

using emul::cpu68k::EAMode;
using emul::cpu68k::CPU68000;

class CPU68000Test : public CPU68000
{
public:
	CPU68000Test(emul::Memory& mem) : CPU68000(mem), Logger("CPU68000Test") {}

	friend class EAModeTester;
};

class TesterBase : public Logger
{
public:
	TesterBase(const char* id) : Logger(id), m_memory(1024)
	{
		EnableLog(Logger::LOG_INFO);
		m_memory.Init(24);
		NewCPU();
		LogPrintf(LOG_INFO, "Start");
	}

	virtual ~TesterBase() {};
	virtual void test() = 0;

	void NewCPU()
	{
		delete m_cpu;
		LogPrintf(LOG_INFO, "Creating New 68000 CPU");
		m_cpu = new CPU68000Test(m_memory);
	}

	using TestFunc = void (TesterBase::*)();

	void RunTest(const char* id, TestFunc func)
	{
		LogPrintf(LOG_INFO, "START %s", id);
		std::invoke(func, this);
		LogPrintf(LOG_INFO, "END %s", id);
	}

protected:
	emul::Memory m_memory;
	CPU68000Test* m_cpu = nullptr;
};

class EAModeTester : public TesterBase
{
public:
	EAModeTester() : TesterBase("TEST_EAMODE") {}

	void ExpectMode(WORD op, EAMode mode)
	{
		if (CPU68000::GetEAMode(op) != mode)
		{
			LogPrintf(LOG_ERROR, "ExpectMode, %04x != %04x", op, mode);
		}
	}

	void testEAModes()
	{
		for (WORD reg = 0; reg < 8; ++reg)
		{
			ExpectMode(0b000000 | reg, EAMode::DRegDirect);
			ExpectMode(0b001000 | reg, EAMode::ARegDirect);
			ExpectMode(0b010000 | reg, EAMode::ARegIndirect);
			ExpectMode(0b011000 | reg, EAMode::ARegIndirectPostinc);
			ExpectMode(0b100000 | reg, EAMode::ARegIndirectPredec);
			ExpectMode(0b101000 | reg, EAMode::ARegIndirectDisp);
			ExpectMode(0b110000 | reg, EAMode::ARegIndirectIndex);
		}

		ExpectMode(0b111000, EAMode::AbsShort);
		ExpectMode(0b111001, EAMode::AbsLong);
		ExpectMode(0b111010, EAMode::PCDisp);
		ExpectMode(0b111011, EAMode::PCIndex);
		ExpectMode(0b111100, EAMode::Immediate);

		ExpectMode(0b111101, EAMode::Invalid);
		ExpectMode(0b111110, EAMode::Invalid);
		ExpectMode(0b111111, EAMode::Invalid);
	}

	void testEAGroups()
	{
		struct EAModeTest {
			const char* id;
			EAMode mode;
		};

		constexpr EAModeTest modes[] = {
			{ "Data Register Direct", EAMode::DRegDirect },
			{ "Address Register Direct", EAMode::ARegDirect },
			{ "Address Register Indirect", EAMode::ARegIndirect },
			{ "Address Register Indirect with Postincrement", EAMode::ARegIndirectPostinc },
			{ "Address Register Indirect with Predecrement", EAMode::ARegIndirectPredec },
			{ "Address Register Indirect with Displacement", EAMode::ARegIndirectDisp },
			{ "Address Register Indirect with Index", EAMode::ARegIndirectIndex },
			{ "Absolute Short", EAMode::AbsShort },
			{ "Absolute Long", EAMode::AbsLong },
			{ "Program Counter with Displacement", EAMode::PCDisp },
			{ "Program Counter with Index", EAMode::PCIndex },
			{ "Immediate", EAMode::Immediate }
		};

		constexpr EAModeTest groups[] = {
			{ "All", EAMode::GroupAll },
			{ "Data", EAMode::GroupData },
			{ "Memory Alterable", EAMode::GroupMemAlt },
			{ "Data Alterable", EAMode::GroupDataAlt },
			{ "Data Address Alterable", EAMode::GroupDataAddrAlt },
			{ "Control", EAMode::GroupControl },
			{ "Control Alterable", EAMode::GroupControlAlt },
			{ "Control Alterable with Predecrement", EAMode::GroupControlAltPredec },
			{ "Control Alterable with Postincrement", EAMode::GroupControlAltPostinc }
		};

		constexpr int testVectors[12][9]{
			{ 1, 1, 0, 1, 1, 0, 0, 0, 0 }, // DataRegDirect
			{ 1, 0, 0, 0, 1, 0, 0, 0, 0 }, // AddrRegDirect
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1 }, // AddrRegIndirect
			{ 1, 1, 1, 1, 1, 0, 0, 0, 1 }, // AddrRegIndirectPostincrement
			{ 1, 1, 1, 1, 1, 0, 0, 1, 0 }, // AddrRegIndirectPredecrement
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1 }, // AddrRegIndirectDisplacement
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1 }, // AddrRegIndirectIndex
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1 }, // AbsoluteShort
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1 }, // AbsoluteLong
			{ 1, 1, 0, 0, 0, 1, 0, 0, 0 }, // ProgramCounterDisplacement
			{ 1, 1, 0, 0, 0, 1, 0, 0, 0 }, // ProgramCounterIndex
			{ 1, 1, 0, 0, 0, 0, 0, 0, 0 }, // Immediate
		};

		for (int m = 0; m < 12; ++m)
		{
			LogPrintf(LOG_INFO, "EA Mode: %s", modes[m].id);
			EAMode currMode = modes[m].mode;

			for (int g = 0; g < 9; ++g)
			{
				EAMode group = groups[g].mode;

				bool modeInGroup = ((WORD)group & (WORD)currMode);
				bool expected = testVectors[m][g];
				if (modeInGroup != expected)
				{
					LogPrintf(LOG_ERROR, "Mode %s should %s in group %s",
						modes[m].id,
						expected ? "BE" : "NOT be",
						groups[g].id);
				}
			}
		}
	}

	virtual void test() override
	{
		RunTest("Test EA Modes", (TestFunc)(&EAModeTester::testEAModes));
		RunTest("Test EA Groups", (TestFunc)(&EAModeTester::testEAGroups));
	}
};

void LogCallback(const char* str)
{
	fprintf(stdout, str);
}

void testCPU()
{
	Logger::RegisterLogCallback(LogCallback);

	{
		EAModeTester tester;
		tester.test();
	}

}
