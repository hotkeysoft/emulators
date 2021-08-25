#include "Device8237.h"
#include "PortAggregator.h"

namespace dma
{
	DMAChannel::DMAChannel(Device8237* parent, WORD id, const char* label) :
		Logger(label),
		m_parent(parent),
		m_id(id),
		m_address(0x1234),
		m_count(0x2345)
	{
	}

	void DMAChannel::Init()
	{
		Connect(m_parent->GetBaseAdress() + (m_id * 2), static_cast<PortConnector::INFunction>(&DMAChannel::ADDR_IN));
		Connect(m_parent->GetBaseAdress() + (m_id * 2), static_cast<PortConnector::OUTFunction>(&DMAChannel::ADDR_OUT));

		Connect(m_parent->GetBaseAdress() + (m_id * 2) + 1, static_cast<PortConnector::INFunction>(&DMAChannel::COUNT_IN));
		Connect(m_parent->GetBaseAdress() + (m_id * 2) + 1, static_cast<PortConnector::OUTFunction>(&DMAChannel::COUNT_OUT));
	}
	void DMAChannel::Reset()
	{
	}

	void DMAChannel::Tick()
	{
		// Fake memory refresh
		//++m_address;
	}

	BYTE DMAChannel::ADDR_IN()
	{
		LogPrintf(LOG_DEBUG, "Read ADDR, value = %02X", m_address);
		return m_address;
	}
	void DMAChannel::ADDR_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write ADDR, value=%02X", value);
		m_address = value;
	}

	BYTE DMAChannel::COUNT_IN()
	{
		LogPrintf(LOG_DEBUG, "Read COUNT");
		return m_count;
	}

	void DMAChannel::COUNT_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write COUNT, value=%02X", value);
		m_count = value;
	}

	Device8237::Device8237(WORD baseAddress) :
		Logger("dma"),
		m_channel0(this, 0, "dma.0"),
		m_channel1(this, 1, "dma.1"),
		m_channel2(this, 2, "dma.2"),
		m_channel3(this, 3, "dma.3"),
		m_baseAddress(baseAddress)
	{
		Reset();
	}

	void Device8237::EnableLog(bool enable, SEVERITY minSev)
	{
		Logger::EnableLog(enable, minSev);

		m_channel0.EnableLog(enable, minSev);
		m_channel1.EnableLog(enable, minSev);
		m_channel2.EnableLog(enable, minSev);
		m_channel3.EnableLog(enable, minSev);
	}

	void Device8237::Reset()
	{

	}

	void Device8237::Init()
	{
		// Registers port 0..7
		m_channel0.Init();
		m_channel1.Init();
		m_channel2.Init();
		m_channel3.Init();

		// Ports 8..16, control, etc.

		// base+8: Command Register (write), Status Register (read)
		Connect(GetBaseAdress() + 8, static_cast<PortConnector::INFunction>(&Device8237::ReadStatus));
		Connect(GetBaseAdress() + 8, static_cast<PortConnector::OUTFunction>(&Device8237::WriteCommand));

		// base+9: Request (write)
		Connect(GetBaseAdress() + 9, static_cast<PortConnector::OUTFunction>(&Device8237::WriteRequest));

		// base+10: Single Mask Bit (write)
		Connect(GetBaseAdress() + 10, static_cast<PortConnector::OUTFunction>(&Device8237::WriteSingleMaskBit));

		// base+11: Mode (write)
		Connect(GetBaseAdress() + 11, static_cast<PortConnector::OUTFunction>(&Device8237::WriteMode));

		// base+12: Clear Byte Flip-flop (write)
		Connect(GetBaseAdress() + 12, static_cast<PortConnector::OUTFunction>(&Device8237::ClearFlipFlop));

		// base+13: Master Clear (write), Temporary Register (read)
		Connect(GetBaseAdress() + 13, static_cast<PortConnector::INFunction>(&Device8237::ReadTemp));
		Connect(GetBaseAdress() + 13, static_cast<PortConnector::OUTFunction>(&Device8237::MasterClear));

		// base+15: All Mask Bits (write)
		Connect(GetBaseAdress() + 15, static_cast<PortConnector::OUTFunction>(&Device8237::WriteAllMaskBits));
	}

	void Device8237::Tick()
	{
		static size_t div = 0;

		if (div++ == 15)
		{
			div = 0;
			m_channel0.Tick();
		}
	}

	bool Device8237::ConnectTo(emul::PortAggregator& dest)
	{
		// Connect sub devices
		dest.Connect(m_channel0);
		dest.Connect(m_channel1);
		dest.Connect(m_channel2);
		dest.Connect(m_channel3);
		return PortConnector::ConnectTo(dest);
	}

	BYTE Device8237::ReadStatus()
	{
		LogPrintf(LOG_DEBUG, "Read Status");
		return 0;
	}
	BYTE Device8237::ReadTemp()
	{
		LogPrintf(LOG_DEBUG, "Read Temporary");
		return 0;
	}

	void Device8237::WriteCommand(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write Command, value=%02X", value);
		m_commandReg = value;

		LogPrintf(LOG_DEBUG, "Controller: %s", (m_commandReg & CMD_DISABLE) ? "Disabled" : "Enabled");
		if (m_commandReg & CMD_DISABLE)
			return;

		LogPrintf(LOG_DEBUG, "Memory to Memory: %s", (m_commandReg & CMD_MEM2MEM) ? "Enabled" : "Disabled");

		if (m_commandReg & CMD_MEM2MEM)
		{
			LogPrintf(LOG_DEBUG, "Channel 0 Address Hold: %s", (m_commandReg & CMD_DMA0_HOLD) ? "Enabled" : "Disabled");
		}

		if (!(m_commandReg & CMD_MEM2MEM))
		{
			LogPrintf(LOG_DEBUG, "Timing: %s", (m_commandReg & CMD_TIMING) ? "Compressed" : "Normal");
		}

		LogPrintf(LOG_DEBUG, "Priority: %s", (m_commandReg & CMD_PRIORITY) ? "Rotating" : "Fixed");

		if (!(m_commandReg & CMD_TIMING))
		{
			LogPrintf(LOG_DEBUG, "Write Selection: %s", (m_commandReg & CMD_WRITE_SEL) ? "Extended" : "Late");
		}

		LogPrintf(LOG_DEBUG, "DREQ Sense Active: %s", (m_commandReg & CMD_DREQ_SENSE) ? "LOW" : "HIGH");
		LogPrintf(LOG_DEBUG, "DACK Sense Active: %s", (m_commandReg & CMD_DACK_SENSE) ? "LOW" : "HIGH");
	}
	void Device8237::WriteRequest(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write Request, value=%02X", value);
	}
	void Device8237::WriteSingleMaskBit(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write Single Mask Bit, value=%02X", value);
	}
	void Device8237::WriteMode(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write Mode, value=%02X", value);
	}
	void Device8237::ClearFlipFlop(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Clear Byte Flip-flop, value=%02X", value);
	}
	void Device8237::MasterClear(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Master Clear, value=%02X", value);
	}
	void Device8237::WriteAllMaskBits(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write All Mask Bits, value=%02X", value);
	}

	void Device8237::DMARequest(size_t channel, bool state)
	{
		if (channel > 3) throw std::exception("invalid dma channel");
		// TODO
	}
	bool Device8237::DMAAcknowledged(size_t channel)
	{
		if (channel > 3) throw std::exception("invalid dma channel");
		// TODO
		return false;
	}

}
