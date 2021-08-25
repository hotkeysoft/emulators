#pragma once

#include "Common.h"
#include "PortConnector.h"

using emul::PortConnector;
class emul::PortAggregator;
using emul::BYTE;
using emul::WORD;

namespace dma
{
	class Device8237;

	class DMAChannel : public PortConnector
	{
	public:
		DMAChannel(Device8237* parent, WORD id, const char* label);

		DMAChannel() = delete;
		DMAChannel(const DMAChannel&) = delete;
		DMAChannel& operator=(const DMAChannel&) = delete;
		DMAChannel(DMAChannel&&) = delete;
		DMAChannel& operator=(DMAChannel&&) = delete;

		void Init();
		void Reset();

		void Tick();

		BYTE ADDR_IN();
		void ADDR_OUT(BYTE value);

		BYTE COUNT_IN();
		void COUNT_OUT(BYTE value);

	private:
		Device8237* m_parent;
		WORD m_id;

		WORD m_address;
		WORD m_count;
	};

	class Device8237 : public PortConnector
	{
	public:
		Device8237(WORD baseAddress);

		Device8237() = delete;
		Device8237(const Device8237&) = delete;
		Device8237& operator=(const Device8237&) = delete;
		Device8237(Device8237&&) = delete;
		Device8237& operator=(Device8237&&) = delete;

		WORD GetBaseAdress() { return m_baseAddress; }

		virtual void EnableLog(bool enable, SEVERITY minSev = LOG_INFO);

		void Init();
		void Reset();

		void Tick();

		void DMARequest(size_t channel, bool state);
		bool DMAAcknowledged(size_t channel);

		virtual bool ConnectTo(emul::PortAggregator& dest);

	protected:
		const WORD m_baseAddress;

		enum CMD {
			CMD_DACK_SENSE = 128,
			CMD_DREQ_SENSE = 64,
			CMD_WRITE_SEL = 32,
			CMD_PRIORITY = 16,
			CMD_TIMING = 8,
			CMD_DISABLE = 4,
			CMD_DMA0_HOLD = 2,
			CMD_MEM2MEM = 1,
		};

		BYTE ReadStatus();
		BYTE ReadTemp();

		void WriteCommand(BYTE value);
		void WriteRequest(BYTE value);
		void WriteSingleMaskBit(BYTE value);
		void WriteMode(BYTE value);
		void ClearFlipFlop(BYTE value);
		void MasterClear(BYTE value);
		void WriteAllMaskBits(BYTE value);

		DMAChannel m_channel0;
		DMAChannel m_channel1;
		DMAChannel m_channel2;
		DMAChannel m_channel3;

		BYTE m_commandReg;
	};
}
