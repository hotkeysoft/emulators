#include "stdafx.h"
#include "CPUZ80Test.h"

#include "test/daa.h"
using namespace cpuTestZ80;

namespace emul
{
	void CPUZ80Test::TestDAA(bool checkFlags)
	{
		LogPrintf(LOG_INFO, "DAA Test");

		const int total = 2048;
		int fail = 0;
		for (WORD i = 0; i < total; ++i)
		{
			if (!TestDAA(i, checkFlags))
			{
				++fail;
			}
		}

		if (fail)
		{
			LogPrintf(LOG_ERROR, "TEST FAIL [%d/%d]", fail, total);
		}

		LogPrintf(LOG_INFO, "TEST PASS [%d/%d]", total-fail, total);
	}

	bool CPUZ80Test::TestDAA(WORD i, bool checkFlags)
	{
		bool in_n = GetBit(i, 10);
		bool in_c = GetBit(i, 9);
		bool in_h = GetBit(i, 8);
		BYTE in_a = GetLByte(i);

		bool out_n = daaTable[i][0];
		bool out_c = daaTable[i][1];
		bool out_h = daaTable[i][2];
		BYTE out_a = daaTable[i][3];

		LogPrintf(LOG_TRACE, "N %d C %d H %d %02X N %d C %d H %d %02X",
			in_n, in_c, in_h, in_a, out_n, out_c, out_h, out_a);

		return TestNCHA(i, checkFlags, in_n, in_c, in_h, in_a, out_n, out_c, out_h, out_a, [&]() { DAA(); });
	}

	bool CPUZ80Test::TestNCHA(int num, bool checkFlags, bool in_n, bool in_c, bool in_h, BYTE in_a,
		bool out_n, bool out_c, bool out_h, BYTE out_a,
		std::function<void()>func)
	{
		SetFlag(FLAG_N, in_n);
		SetFlag(FLAG_CY, in_c);
		SetFlag(FLAG_H, in_h);
		m_reg.A = in_a;

		func();

		if ((m_reg.A != out_a) || (checkFlags && (
			(GetFlag(FLAG_N) != out_n) ||
			(GetFlag(FLAG_CY) != out_c)  ||
			(GetFlag(FLAG_H) != out_h))))
		{
			LogPrintf(LOG_ERROR, "TEST #[%04d]N[%d]C[%d]H[%d]A[%02X] FAILED [EXPECTED/ACTUAL]: "
				"N[%d/%d]"
				"C[%d/%d]"
				"H[%d/%d]"
				"A[%02X/%02X]",
				num,
				in_n, in_c, in_h, in_a,
				out_n, GetFlag(FLAG_N),
				out_c, GetFlag(FLAG_CY),
				out_h, GetFlag(FLAG_H),
				out_a, m_reg.A);
			return false;
		}
		return true;
	}

}