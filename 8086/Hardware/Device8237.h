#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>
#include <CPU/Memory.h>

using emul::PortConnector;
using emul::BYTE;
using emul::WORD;

namespace dma
{
	static const BYTE DMA_CHANNELS = 4;

	class Device8237;

	enum class OPERATION { VERIFY, READ, WRITE, INVALID };

	class DMAChannel : public PortConnector
	{
	public:
		DMAChannel(Device8237* parent, emul::Memory& memory, BYTE id, std::string label);

		DMAChannel() = delete;
		DMAChannel(const DMAChannel&) = delete;
		DMAChannel& operator=(const DMAChannel&) = delete;
		DMAChannel(DMAChannel&&) = delete;
		DMAChannel& operator=(DMAChannel&&) = delete;

		void Init(BYTE addressShift);
		void Reset();

		void SetCascadedDevice(Device8237* primary);

		void Tick();

		BYTE ReadAddress();
		void WriteAddress(BYTE value);

		BYTE ReadCount();
		void WriteCount(BYTE value);

		void SetMode(BYTE mode);

		void SetPage(BYTE page) { m_page = page; }

		OPERATION GetOperation() { return m_operation; }
		void DMAOperation(BYTE& value);

	private:
		Device8237* m_parent;
		emul::Memory& m_memory;
		BYTE m_id;
		BYTE m_addressShift = 0;

		bool m_cascade = false;
		Device8237* m_cascadedDevice = nullptr;

		enum MODE {
			MODE_M1 = 128,
			MODE_M0 = 64,
			MODE_ADDR_DECREMENT = 32,
			MODE_AUTO_INIT = 16,
			MODE_OP1 = 8,
			MODE_OP0 = 4
		};
		BYTE m_mode = 0;
		OPERATION m_operation = OPERATION::INVALID;
		bool m_decrement = false;
		bool m_autoInit = false;
		bool m_terminalCount = false;

		WORD m_baseCount = 0;
		WORD m_count = 0;

		WORD m_baseAddress = 0;
		WORD m_address = 0;

		WORD m_page = 0;
	};

	class Device8237 : public PortConnector
	{
	public:
		Device8237(const char* id, WORD baseAddress, emul::Memory& m_memory);
		Device8237(WORD baseAddress, emul::Memory& m_memory);

		Device8237() = delete;
		Device8237(const Device8237&) = delete;
		Device8237& operator=(const Device8237&) = delete;
		Device8237(Device8237&&) = delete;
		Device8237& operator=(Device8237&&) = delete;

		WORD GetBaseAddress() { return m_baseAddress; }

		virtual void EnableLog(SEVERITY minSev = LOG_INFO);

		// Address shift will shift all addresses and ports one bit, for 16 bit DMA transfers
		// (used on AT+)
		void Init(BYTE addressShift = 0);
		void Reset();

		void SetCascadedDevice(BYTE channel, Device8237* device);

		void Tick();

		void DMARequest(BYTE channel, bool state);
		bool DMAAcknowledged(BYTE channel);

		bool GetByteFlipFlop(bool toggle = false);
		bool IsDisabled() { return (m_commandReg & CMD_DISABLE); }

		bool GetTerminalCount(BYTE channel);
		void SetTerminalCount(BYTE channel, bool tc = true);

		DMAChannel& GetChannel(BYTE channel) { return m_channels[channel & 3]; }

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

		std::array<DMAChannel, DMA_CHANNELS> m_channels;
		std::array<bool, DMA_CHANNELS> m_dmaRequests = {};

		BYTE m_commandReg;
		BYTE m_statusReg;
		BYTE m_requestReg;
		BYTE m_tempReg;
		BYTE m_maskReg;

		bool m_byteFlipFlop;
	};
}
