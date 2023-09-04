#include "stdafx.h"

#include "../CPU/CPU68000.h"
#include <functional>

using emul::cpu68k::EAMode;
using emul::cpu68k::CPU68000;
using emul::GetLByte;
using emul::GetLWord;

class CPU68000Test : public CPU68000
{
public:
	CPU68000Test(emul::Memory& mem) : CPU68000(mem), Logger("CPU68000Test") {}

	friend class EAModeTester;
	friend class RegisterTester;
	friend class BCDTester;
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

	template <typename T>
	void ExpectEqual(T expect, T actual, const char* msg = nullptr)
	{
		static_assert(
			std::is_same<T, emul::BYTE>::value ||
			std::is_same<T, emul::WORD>::value ||
			std::is_same<T, emul::DWORD>::value);
		if (expect != actual)
		{
			LogPrintf(LOG_ERROR, "ExpectEqual failed: expect(%X) != actual(%X) [%s]", expect, actual, msg ? msg : "");
		}
	}

protected:
	emul::Memory m_memory;
	CPU68000Test* m_cpu = nullptr;
};

class RegisterTester : public TesterBase
{
public:
	RegisterTester() : TesterBase("TEST_REG") {}

	constexpr static emul::DWORD initValues[] = {
		0x00008888,
		0x11119999,
		0x2222AAAA,
		0x3333BBBB,
		0x4444CCCC,
		0x5555DDDD,
		0x6666EEEE,
		0x7777FFFF
	};

	void resetRegisters()
	{
		m_cpu->m_reg.ADDR[0] = initValues[0];
		m_cpu->m_reg.ADDR[1] = initValues[1];
		m_cpu->m_reg.ADDR[2] = initValues[2];
		m_cpu->m_reg.ADDR[3] = initValues[3];
		m_cpu->m_reg.ADDR[4] = initValues[4];
		m_cpu->m_reg.ADDR[5] = initValues[5];
		m_cpu->m_reg.ADDR[6] = initValues[6];
		m_cpu->m_reg.ADDR[7] = initValues[7];

		m_cpu->m_reg.DATA[0] = ~initValues[0];
		m_cpu->m_reg.DATA[1] = ~initValues[1];
		m_cpu->m_reg.DATA[2] = ~initValues[2];
		m_cpu->m_reg.DATA[3] = ~initValues[3];
		m_cpu->m_reg.DATA[4] = ~initValues[4];
		m_cpu->m_reg.DATA[5] = ~initValues[5];
		m_cpu->m_reg.DATA[6] = ~initValues[6];
		m_cpu->m_reg.DATA[7] = ~initValues[7];
	}

	void testRegisters()
	{
		resetRegisters();

		for (int i = 0; i < 8; ++i)
		{
			if (m_cpu->m_reg.DATA[i] != ~initValues[i])
			{
				LogPrintf(LOG_ERROR, "Data Register mismatch at index %d", i);
			}
			if (m_cpu->m_reg.ADDR[i] != initValues[i])
			{
				LogPrintf(LOG_ERROR, "Data Register mismatch at index %d", i);
			}
		}
	}

	void testAliasLongR()
	{
		resetRegisters();

		ExpectEqual(initValues[0], m_cpu->m_reg.A0, "A0");
		ExpectEqual(initValues[1], m_cpu->m_reg.A1, "A1");
		ExpectEqual(initValues[2], m_cpu->m_reg.A2, "A2");
		ExpectEqual(initValues[3], m_cpu->m_reg.A3, "A3");
		ExpectEqual(initValues[4], m_cpu->m_reg.A4, "A4");
		ExpectEqual(initValues[5], m_cpu->m_reg.A5, "A5");
		ExpectEqual(initValues[6], m_cpu->m_reg.A6, "A6");
		ExpectEqual(initValues[7], m_cpu->m_reg.A7, "A7");

		ExpectEqual(~initValues[0], m_cpu->m_reg.D0, "D0");
		ExpectEqual(~initValues[1], m_cpu->m_reg.D1, "D1");
		ExpectEqual(~initValues[2], m_cpu->m_reg.D2, "D2");
		ExpectEqual(~initValues[3], m_cpu->m_reg.D3, "D3");
		ExpectEqual(~initValues[4], m_cpu->m_reg.D4, "D4");
		ExpectEqual(~initValues[5], m_cpu->m_reg.D5, "D5");
		ExpectEqual(~initValues[6], m_cpu->m_reg.D6, "D6");
		ExpectEqual(~initValues[7], m_cpu->m_reg.D7, "D7");
	}

	void testAliasWordR()
	{
		resetRegisters();

		ExpectEqual(GetLWord(initValues[0]), m_cpu->m_reg.A0w, "A0w");
		ExpectEqual(GetLWord(initValues[1]), m_cpu->m_reg.A1w, "A1w");
		ExpectEqual(GetLWord(initValues[2]), m_cpu->m_reg.A2w, "A2w");
		ExpectEqual(GetLWord(initValues[3]), m_cpu->m_reg.A3w, "A3w");
		ExpectEqual(GetLWord(initValues[4]), m_cpu->m_reg.A4w, "A4w");
		ExpectEqual(GetLWord(initValues[5]), m_cpu->m_reg.A5w, "A5w");
		ExpectEqual(GetLWord(initValues[6]), m_cpu->m_reg.A6w, "A6w");
		ExpectEqual(GetLWord(initValues[7]), m_cpu->m_reg.A7w, "A7w");

		ExpectEqual(GetLWord(~initValues[0]), m_cpu->m_reg.D0w, "D0w");
		ExpectEqual(GetLWord(~initValues[1]), m_cpu->m_reg.D1w, "D1w");
		ExpectEqual(GetLWord(~initValues[2]), m_cpu->m_reg.D2w, "D2w");
		ExpectEqual(GetLWord(~initValues[3]), m_cpu->m_reg.D3w, "D3w");
		ExpectEqual(GetLWord(~initValues[4]), m_cpu->m_reg.D4w, "D4w");
		ExpectEqual(GetLWord(~initValues[5]), m_cpu->m_reg.D5w, "D5w");
		ExpectEqual(GetLWord(~initValues[6]), m_cpu->m_reg.D6w, "D6w");
		ExpectEqual(GetLWord(~initValues[7]), m_cpu->m_reg.D7w, "D7w");
	}

	void testAliasByteR()
	{
		resetRegisters();

		ExpectEqual(GetLByte(~initValues[0]), m_cpu->m_reg.D0b, "D0b");
		ExpectEqual(GetLByte(~initValues[1]), m_cpu->m_reg.D1b, "D1b");
		ExpectEqual(GetLByte(~initValues[2]), m_cpu->m_reg.D2b, "D2b");
		ExpectEqual(GetLByte(~initValues[3]), m_cpu->m_reg.D3b, "D3b");
		ExpectEqual(GetLByte(~initValues[4]), m_cpu->m_reg.D4b, "D4b");
		ExpectEqual(GetLByte(~initValues[5]), m_cpu->m_reg.D5b, "D5b");
		ExpectEqual(GetLByte(~initValues[6]), m_cpu->m_reg.D6b, "D6b");
		ExpectEqual(GetLByte(~initValues[7]), m_cpu->m_reg.D7b, "D7b");
	}

	void testAliasWrite()
	{
		resetRegisters();

		m_cpu->m_reg.A0 = 0x12345678;
		ExpectEqual((emul::DWORD)0x12345678, m_cpu->m_reg.A0);

		m_cpu->m_reg.A0w = 0xAAAA;
		ExpectEqual((emul::DWORD)0x1234AAAA, m_cpu->m_reg.A0);

		m_cpu->m_reg.D7 = 0xAABBCCDD;
		ExpectEqual((emul::DWORD)0xAABBCCDD, m_cpu->m_reg.D7);

		m_cpu->m_reg.D7w = 0x00FF;
		ExpectEqual((emul::BYTE)0xFF, m_cpu->m_reg.D7b);
		ExpectEqual((emul::WORD)0x00FF, m_cpu->m_reg.D7w);
		ExpectEqual((emul::DWORD)0xAABB00FF, m_cpu->m_reg.D7);

		m_cpu->m_reg.D7b = 0x55;
		ExpectEqual((emul::BYTE)0x55, m_cpu->m_reg.D7b);
		ExpectEqual((emul::WORD)0x0055, m_cpu->m_reg.D7w);
		ExpectEqual((emul::DWORD)0xAABB0055, m_cpu->m_reg.D7);
	}

	virtual void test() override
	{
		RunTest("Test Registers", (TestFunc)(&RegisterTester::testRegisters));
		RunTest("Test alias LONG (Read)", (TestFunc)(&RegisterTester::testAliasLongR));
		RunTest("Test alias WORD (Read)", (TestFunc)(&RegisterTester::testAliasWordR));
		RunTest("Test alias BYTE (Read)", (TestFunc)(&RegisterTester::testAliasByteR));
		RunTest("Test alias Write", (TestFunc)(&RegisterTester::testAliasWrite));
	}

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

class BCDTester : public TesterBase
{
public:
	BCDTester() : TesterBase("TEST_BCD") {}

	std::string flagsToStr(BYTE flags)
	{
		std::string s;

		s.append(1, emul::GetBit(flags, 4) ? 'X' : 'x');
		s.append(1, emul::GetBit(flags, 3) ? 'N' : 'n');
		s.append(1, emul::GetBit(flags, 2) ? 'Z' : 'z');
		s.append(1, emul::GetBit(flags, 1) ? 'V' : 'v');
		s.append(1, emul::GetBit(flags, 0) ? 'C' : 'c');

		return s;
	}

#pragma pack(push, 1)
	struct testRecBCD_xy
	{
		BYTE x;
		BYTE y;
		BYTE flagsPre;
		BYTE result;
		BYTE flagsPost;
	};

	struct testRecNBCD
	{
		BYTE y;
		BYTE flagsPre;
		BYTE result;
		BYTE flagsPost;
	};

#pragma pack(pop)

	enum class TestMode { ABCD, SBCD, NBCD };

	void testTable(const char* tableFile, TestMode mode)
	{
		// Bin file format is (all bytes): x, y, flags(before), result, flags(after)

		// A full table for ABCD/SBCD will have 256 x 256 x 2 entries (for X flag),
		// twice that if we want to test the before/after Z flag
		FILE* f = fopen(tableFile, "rb");

		if (!f)
		{
			LogPrintf(LOG_ERROR, "Can't open table file");
			return;
		}

		testRecBCD_xy record;
		testRecNBCD recordNBCD;
		size_t elements;
		int tests = 0;
		int failValue = 0;
		int failFlags = 0;
		while(true)
		{
			if (mode == TestMode::NBCD)
			{
				elements = fread(&recordNBCD, sizeof(recordNBCD), 1, f);
			}
			else
			{
				elements = fread(&record, sizeof(record), 1, f);
			}

			if (!elements)
				break;

			BYTE dest;

			char op = 0;
			switch (mode)
			{
			case TestMode::ABCD:
				dest = (BYTE)record.x;
				m_cpu->m_reg.flags = record.flagsPre;
				m_cpu->ABCD(dest, (BYTE)record.y, emul::GetBit(record.flagsPre, 4));
				op = '+';
				break;
			case TestMode::SBCD:
				dest = (BYTE)record.y;
				m_cpu->m_reg.flags = record.flagsPre;
				m_cpu->SBCD(dest, (BYTE)record.x, emul::GetBit(record.flagsPre, 4));
				op = '-';
				break;
			case TestMode::NBCD:
				dest = (BYTE)recordNBCD.y;
				m_cpu->m_reg.flags = recordNBCD.flagsPre;
				m_cpu->NBCD(dest, emul::GetBit(recordNBCD.flagsPre, 4));
				break;
			default:
				throw std::exception("Invalid mode");
			}

			if (mode == TestMode::NBCD)
			{
				if (dest != recordNBCD.result)
				{
					LogPrintf(LOG_ERROR, "Test FAIL value: NEG(%02x) != %02x (got %02x)",
						recordNBCD.y,
						recordNBCD.result,
						dest);
				}

				BYTE newFlags = (BYTE)m_cpu->m_reg.flags;
				if (m_cpu->m_reg.flags != recordNBCD.flagsPost)
				{
					LogPrintf(LOG_ERROR, "Test FAIL flags: %s != %s",
						flagsToStr(newFlags).c_str(),
						flagsToStr(recordNBCD.flagsPost).c_str());
					++failFlags;
				}
			}
			else
			{
				if (dest != record.result)
				{
					LogPrintf(LOG_ERROR, "Test FAIL value: %02x %c %02x != %02x (got %02x)",
						record.x,
						op,
						record.y,
						record.result,
						dest);
					++failValue;
				}

				BYTE newFlags = (BYTE)m_cpu->m_reg.flags;
				if (m_cpu->m_reg.flags != record.flagsPost)
				{
					LogPrintf(LOG_ERROR, "Test FAIL flags: %s != %s",
						flagsToStr(newFlags).c_str(),
						flagsToStr(record.flagsPost).c_str());
					++failFlags;
				}
			}

			++tests;
		}
		fclose(f);
		LogPrintf(LOG_INFO, "Total tests: %d", tests);
		LogPrintf(LOG_INFO, "Value fail: %d, flag fail: %d", failValue, failFlags);
	}

	void testABCD()
	{
		testTable("./test/tables/abcdTable.bin", TestMode::ABCD);
	}
	void testSBCD()
	{
		testTable("./test/tables/sbcdTable.bin", TestMode::SBCD);
	}
	void testNBCD()
	{
		testTable("./test/tables/nbcdTable.bin", TestMode::NBCD);
	}
	virtual void test() override
	{
		RunTest("Test ABCD", (TestFunc)(&BCDTester::testABCD));
		RunTest("Test SBCD", (TestFunc)(&BCDTester::testSBCD));
		RunTest("Test NBCD", (TestFunc)(&BCDTester::testNBCD));
	}
};

int testCPU()
{

	{
		EAModeTester tester;
		tester.test();
	}

	{
		RegisterTester tester;
		tester.test();
	}

	{
		BCDTester tester;
		tester.test();
	}

	return 0;
}
