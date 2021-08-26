#include "Common.h"
#include "Device8237.h"
#include "PortAggregator.h"


using emul::GetHiByte;
using emul::GetLowByte;
using emul::SetHiByte;
using emul::SetLowByte;

namespace dma
{

	DMAChannel::DMAChannel(Device8237* parent, WORD id, const char* label) :
		Logger(label),
		m_parent(parent),
		m_id(id),
		m_address(0x0000),
		m_count(0x0000),
		m_mode(0),
		m_decrement(false)
	{
	}

	void DMAChannel::Init()
	{
		Connect(m_parent->GetBaseAdress() + (m_id * 2), static_cast<PortConnector::INFunction>(&DMAChannel::ReadAddress));
		Connect(m_parent->GetBaseAdress() + (m_id * 2), static_cast<PortConnector::OUTFunction>(&DMAChannel::WriteAddress));

		Connect(m_parent->GetBaseAdress() + (m_id * 2) + 1, static_cast<PortConnector::INFunction>(&DMAChannel::ReadCount));
		Connect(m_parent->GetBaseAdress() + (m_id * 2) + 1, static_cast<PortConnector::OUTFunction>(&DMAChannel::WriteCount));
	}

	void DMAChannel::Reset()
	{
	}

	void DMAChannel::Tick()
	{
		if (!m_parent->IsEnabled())
			return;

		// Fake memory refresh
		--m_count;

		if (m_decrement)
		{
			--m_address;
		}
		else
		{
			++m_address;
		}
	}

	BYTE DMAChannel::ReadAddress()
	{
		bool loHi = m_parent->GetByteFlipFlop(true);
		LogPrintf(LOG_DEBUG, "Read ADDRESS, value = %02X, lowHi=%d", m_address, loHi);
		return (loHi ? GetHiByte(m_address) : GetLowByte(m_address));
	}
	void DMAChannel::WriteAddress(BYTE value)
	{
		bool loHi = m_parent->GetByteFlipFlop(true);
		LogPrintf(LOG_DEBUG, "Write ADDRESS, value=%02X, lowHi=%d", value, loHi);
		loHi ? SetHiByte(m_address, value) : SetLowByte(m_address, value);
	}

	BYTE DMAChannel::ReadCount()
	{
		bool loHi = m_parent->GetByteFlipFlop(true);
		LogPrintf(LOG_DEBUG, "Read COUNT, value=%02X, lowHi=%d", m_count, loHi);
		return (loHi ? GetHiByte(m_count) : GetLowByte(m_count));
	}

	void DMAChannel::WriteCount(BYTE value)
	{
		bool loHi = m_parent->GetByteFlipFlop(true);
		LogPrintf(LOG_DEBUG, "Write COUNT, value=%02X, lowHi=%d", value, loHi);
		loHi ? SetHiByte(m_count, value) : SetLowByte(m_count, value);
	}

	void DMAChannel::SetMode(BYTE mode)
	{
		LogPrintf(LOG_DEBUG, "Set Mode, mode=%02X", mode);
		m_mode = mode;

		switch (m_mode & (MODE_M1 | MODE_M0))
		{
		case 0: LogPrintf(LOG_INFO, "Mode: DEMAND");
			break;
		case MODE_M0: LogPrintf(LOG_INFO, "Mode: SINGLE");
			break;
		case MODE_M1: LogPrintf(LOG_INFO, "Mode: BLOCK");
			break;
		case MODE_M1 | MODE_M0: LogPrintf(LOG_INFO, "Mode: CASCADE");
			throw std::exception("not implemented");
		default:
			throw std::exception("not possible");
		}

		switch (m_mode & (MODE_OP1 | MODE_OP0))
		{
		case 0: LogPrintf(LOG_INFO, "Operation: VERIFY Transfer");
			break;
		case MODE_OP0: LogPrintf(LOG_INFO, "Operation: WRITE Transfer");
			break;
		case MODE_OP1: LogPrintf(LOG_INFO, "Operation: READ Transfer");
			break;
		case MODE_OP1 | MODE_OP0: LogPrintf(LOG_ERROR, "Invalid operation");
			break;
		default:
			throw std::exception("not possible");
		}

		LogPrintf(LOG_INFO, "Autoinitialization: %s", (m_mode & MODE_AUTO_INIT) ? "Enabled" : "Disabled");

		m_decrement = (m_mode & MODE_ADDR_DECREMENT);
		LogPrintf(LOG_INFO, "Address %s", m_decrement ? "Decrement" : "Increment");

	}

	Device8237::Device8237(WORD baseAddress) :
		Logger("dma"),
		m_channel0(this, 0, "dma.0"),
		m_channel1(this, 1, "dma.1"),
		m_channel2(this, 2, "dma.2"),
		m_channel3(this, 3, "dma.3"),
		m_baseAddress(baseAddress),
		m_commandReg(0xFF),
		m_disabled(false),
		m_byteFlipFlop(false)
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
		m_commandReg = 0xFF;
		m_byteFlipFlop = false;
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

		m_disabled = (m_commandReg & CMD_DISABLE);
		LogPrintf(LOG_INFO, "Controller: %s", m_disabled ? "Disabled" : "Enabled");
		if (m_commandReg & CMD_DISABLE)
			return;

		LogPrintf(LOG_INFO, "Memory to Memory: %s", (m_commandReg & CMD_MEM2MEM) ? "Enabled" : "Disabled");

		if (m_commandReg & CMD_MEM2MEM)
		{
			LogPrintf(LOG_INFO, "Channel 0 Address Hold: %s", (m_commandReg & CMD_DMA0_HOLD) ? "Enabled" : "Disabled");
		}

		if (!(m_commandReg & CMD_MEM2MEM))
		{
			LogPrintf(LOG_INFO, "Timing: %s", (m_commandReg & CMD_TIMING) ? "Compressed" : "Normal");
		}

		LogPrintf(LOG_INFO, "Priority: %s", (m_commandReg & CMD_PRIORITY) ? "Rotating" : "Fixed");

		if (!(m_commandReg & CMD_TIMING))
		{
			LogPrintf(LOG_INFO, "Write Selection: %s", (m_commandReg & CMD_WRITE_SEL) ? "Extended" : "Late");
		}

		LogPrintf(LOG_INFO, "DREQ Sense Active: %s", (m_commandReg & CMD_DREQ_SENSE) ? "LOW" : "HIGH");
		LogPrintf(LOG_INFO, "DACK Sense Active: %s", (m_commandReg & CMD_DACK_SENSE) ? "LOW" : "HIGH");
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
		switch (value & 3)
		{
		case 0: m_channel0.SetMode(value);
			break;
		case 1: m_channel1.SetMode(value);
			break;
		case 2: m_channel2.SetMode(value);
			break;
		case 3: m_channel3.SetMode(value);
			break;
		default:
			throw std::exception("not possible");
		}
	}
	void Device8237::ClearFlipFlop(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Clear Byte Flip-flop, value=%02X", value);
		m_byteFlipFlop = false;
	}
	void Device8237::MasterClear(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Master Clear, value=%02X", value);
		m_byteFlipFlop = false;
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

	bool Device8237::GetByteFlipFlop(bool toggle)
	{
		bool ret = m_byteFlipFlop;
		if (toggle)
		{
			m_byteFlipFlop = !m_byteFlipFlop;
		}
		return ret;
	}

}
