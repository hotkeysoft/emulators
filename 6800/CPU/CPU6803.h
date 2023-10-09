#pragma once

#include "CPU/CPU6800.h"
#include <CPU/CPUInfo.h>
#include <EdgeDetectLatch.h>

#undef IN
#undef OUT

namespace emul
{
	static const char* CPUID_6803 = "6803";

	class CPU6803 : public CPU6800
	{
	public:
		CPU6803(Memory& memory, bool internalRAM);
		virtual ~CPU6803();

		virtual void Init() override;

		virtual void Dump() override;

		virtual void Reset() override;
		virtual void Reset(ADDRESS overrideAddress) override { CPU6800::Reset(overrideAddress); }

	protected:
		MemoryBlock* m_internalRAM = nullptr;

		enum class IORegister
		{
			Port1DataDirection = 0x00,
			Port2DataDirection = 0x01,

			Port1Data = 0x02,
			Port2Data = 0x03,

			TimerControlStatus = 0x08,
			CounterHigh = 0x09,
			CounterLow = 0x0A,

			OutputCompareHigh = 0x0B,
			OutputCompareLow = 0x0C,

			InputCaptureHigh = 0x0D,
			InputCaptureLow = 0x0E,

			RateModeControl = 0x10,
			TxRxControlStatus = 0x11,
			RxData = 0x12,
			TxData = 0x13,

			RAMControl = 0x14,

			_Reserved15, _Reserved16, _Reserved17, _Reserved18, _Reserved19,
			_Reserved1A, _Reserved1B, _Reserved1C, _Reserved1D, _Reserved1E, _Reserved1F,

			_REG_COUNT
		};

		virtual BYTE MemRead8(ADDRESS address) override;
		virtual void MemWrite8(ADDRESS address, BYTE value) override;

		BYTE IORead(IORegister reg);
		void IOWrite(IORegister reg, BYTE value);

		// Hardware vectors
		static constexpr ADDRESS ADDR_ICF = 0xFFF6; // Input Capture
		static constexpr ADDRESS ADDR_OCF = 0xFFF4; // Output Capture
		static constexpr ADDRESS ADDR_TCF = 0xFFF2; // Timer Overflow
		static constexpr ADDRESS ADDR_SCI = 0xFFF0; // Serial Communication Interface

		void MUL();

		friend class Monitor6800;
	};
}
