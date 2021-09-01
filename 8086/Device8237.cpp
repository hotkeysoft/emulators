#include "Common.h"
#include "Device8237.h"
#include "PortAggregator.h"

using emul::GetHiByte;
using emul::GetLowByte;
using emul::SetHiByte;
using emul::SetLowByte;
using emul::SetBit;

namespace dma
{

	DMAChannel::DMAChannel(Device8237* parent, emul::Memory& memory, BYTE id, const char* label) :
		Logger(label),
		m_parent(parent),
		m_memory(memory),
		m_id(id),
		m_baseCount(0x0000),
		m_count(0x0000),
		m_baseAddress(0x0000),
		m_address(0x0000),
		m_mode(0),
		m_decrement(false),
		m_autoInit(false)
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
		m_baseCount = 0x0000;
		m_count = 0x0000;
		m_baseAddress = 0x0000;
		m_address = 0x0000;
		m_mode = 0;
		m_decrement = false;
		m_autoInit = false;
	}

	void DMAChannel::Tick()
	{
		if (m_parent->IsDisabled())
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

		if (m_count == 0)
		{
			LogPrintf(LOG_INFO, "Channel %d, Count done", m_id);

			if (m_autoInit)
			{
				m_count = m_baseCount;
				m_address = m_baseAddress;
			}
			m_parent->SetTerminalCount(m_id);
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

		if (loHi)
		{
			LogPrintf(LOG_INFO, "Write ADDRESS, value=%04X (%d)", m_address, m_address);
			m_baseAddress = m_address;
		}
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

		if (loHi)
		{
			LogPrintf(LOG_INFO, "Write COUNT, value=%04X (%d)", m_count, m_count);
			m_baseCount = m_count;
		}
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

	void DMAChannel::DMAWrite(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "DMA Write, value=%02X @ Address %04x", value, m_address);

		if ((m_mode & (MODE_OP1 | MODE_OP0)) == MODE_OP0) // WRITE Transfer
		{
			// TODO: Bank, port 80
			m_memory.Write(m_address, value);
		}
		else 
		{
			throw std::exception("DMAWrite: Mode not supported");
		}

		Tick();
	}

	Device8237::Device8237(WORD baseAddress, emul::Memory& memory) :
		Logger("dma"),
		m_channel0(this, memory, 0, "dma.0"),
		m_channel1(this, memory, 1, "dma.1"),
		m_channel2(this, memory, 2, "dma.2"),
		m_channel3(this, memory, 3, "dma.3"),
		m_baseAddress(baseAddress),
		m_commandReg(0),
		m_statusReg(0),
		m_requestReg(0),
		m_tempReg(0),
		m_maskReg(0xFF),
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
		m_commandReg = 0;
		m_statusReg = 0;
		m_requestReg =0;
		m_tempReg = 0;
		m_maskReg = 0xFF;
		m_byteFlipFlop = false;

		DMARequests[0] = false;
		DMARequests[1] = false;
		DMARequests[2] = false;
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

		// TODO: Fake memory refresh until everything is connected
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
		LogPrintf(LOG_INFO, "Read Status, value=%02X", m_statusReg);
		BYTE ret = m_statusReg;
		// Reset TC bits
		m_statusReg &= 0xF0;
		return ret;
	}
	BYTE Device8237::ReadTemp()
	{
		LogPrintf(LOG_INFO, "Read Temporary, value=%02X", m_tempReg);
		return m_tempReg;
	}

	void Device8237::WriteCommand(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write Command, value=%02X", value);
		m_commandReg = value;

		LogPrintf(LOG_INFO, "Controller: %s", IsDisabled() ? "Disabled" : "Enabled");
		if (IsDisabled())
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
		BYTE channel = value & 3;
		bool bit = (value & 4);
		SetBit(m_maskReg, channel, bit);
		LogPrintf(LOG_INFO, "Set Channel %d mask bit: %s", channel, bit ? "SET" : "CLEAR");
	}
	void Device8237::WriteAllMaskBits(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write All Mask Bits, value=%02X", value);
		m_maskReg = (value & 0xF0);
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

	void Device8237::DMARequest(BYTE channel, bool state)
	{
		if (channel > 2) throw std::exception("invalid dma channel");
		DMARequests[channel % 3] = state;
	}
	bool Device8237::DMAAcknowledged(BYTE channel)
	{
		if (channel > 2) throw std::exception("invalid dma channel");
		// TODO
		bool ack = DMARequests[channel % 3];
		DMARequests[channel % 3] = false;		
		return ack;
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

	void Device8237::SetTerminalCount(BYTE channel)
	{
		channel &= 3;
		m_statusReg |= (1 << channel);
	}

	void Device8237::DMAWrite(BYTE channel, BYTE data)
	{
		switch (channel & 3)
		{
		case 0: m_channel0.DMAWrite(data);
			break;
		case 1: m_channel1.DMAWrite(data);
			break;
		case 2: m_channel2.DMAWrite(data);
			break;
		case 3: m_channel3.DMAWrite(data);
			break;
		default:
			throw std::exception("not possible");
		}
	}
}
