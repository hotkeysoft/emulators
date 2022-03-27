#include "stdafx.h"

#include "Device8237.h"

using emul::GetHByte;
using emul::GetLByte;
using emul::SetHByte;
using emul::SetLByte;
using emul::SetBit;

namespace dma
{

	DMAChannel::DMAChannel(Device8237* parent, emul::Memory& memory, BYTE id, std::string label) :
		Logger(label.c_str()),
		m_parent(parent),
		m_memory(memory),
		m_id(id)
	{
	}
	
	void DMAChannel::Init(BYTE addressShift)
	{
		m_addressShift = addressShift;

		WORD address = m_parent->GetBaseAddress() + (m_id << (m_addressShift + 1));

		Connect(address, static_cast<PortConnector::INFunction>(&DMAChannel::ReadAddress));
		Connect(address, static_cast<PortConnector::OUTFunction>(&DMAChannel::WriteAddress));

		Connect(address + (1 << m_addressShift), static_cast<PortConnector::INFunction>(&DMAChannel::ReadCount));
		Connect(address + (1 << m_addressShift), static_cast<PortConnector::OUTFunction>(&DMAChannel::WriteCount));
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
		m_terminalCount = false;
	}

	void DMAChannel::Tick()
	{
		if (m_parent->IsDisabled())
			return;

		--m_count;

		if (m_decrement)
		{
			--m_address;
		}
		else
		{
			++m_address;
		}

		if (m_count == 0xFFFF)
		{
			LogPrintf(LOG_DEBUG, "Channel %d, Count done", m_id);

			// TODO: Terminalcount & correct states
			if (m_autoInit)
			{
				m_count = m_baseCount;
				m_address = m_baseAddress;
			}
			else
			{
				m_terminalCount = true;
				m_parent->SetTerminalCount(m_id);
			}
		}
	}

	BYTE DMAChannel::ReadAddress()
	{
		bool loHi = m_parent->GetByteFlipFlop(true);
		LogPrintf(LOG_DEBUG, "Read ADDRESS, value = %02X, lowHi=%d", m_address, loHi);
		return (loHi ? GetHByte(m_address) : GetLByte(m_address));
	}
	void DMAChannel::WriteAddress(BYTE value)
	{
		bool loHi = m_parent->GetByteFlipFlop(true);
		LogPrintf(LOG_DEBUG, "Write ADDRESS, value=%02X, lowHi=%d", value, loHi);
		loHi ? SetHByte(m_address, value) : SetLByte(m_address, value);

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
		return (loHi ? GetHByte(m_count) : GetLByte(m_count));
	}

	void DMAChannel::WriteCount(BYTE value)
	{
		bool loHi = m_parent->GetByteFlipFlop(true);
		LogPrintf(LOG_DEBUG, "Write COUNT, value=%02X, lowHi=%d", value, loHi);
		loHi ? SetHByte(m_count, value) : SetLByte(m_count, value);

		if (loHi)
		{
			LogPrintf(LOG_INFO, "Write COUNT, value=%04X (%d)", m_count, m_count);
			m_baseCount = m_count;
			m_terminalCount = false;
			m_parent->SetTerminalCount(m_id, false);
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
			m_operation = OPERATION::VERIFY;
			break;
		case MODE_OP0: LogPrintf(LOG_INFO, "Operation: WRITE Transfer");
			m_operation = OPERATION::WRITE;
			break;
		case MODE_OP1: LogPrintf(LOG_INFO, "Operation: READ Transfer");
			m_operation = OPERATION::READ;
			break;
		case MODE_OP1 | MODE_OP0: LogPrintf(LOG_ERROR, "Invalid operation");
			m_operation = OPERATION::INVALID;
			break;
		default:
			throw std::exception("not possible");
		}

		LogPrintf(LOG_INFO, "Autoinitialization: %s", (m_mode & MODE_AUTO_INIT) ? "Enabled" : "Disabled");

		m_decrement = (m_mode & MODE_ADDR_DECREMENT);
		LogPrintf(LOG_INFO, "Address %s", m_decrement ? "Decrement" : "Increment");

		m_terminalCount = false;
		m_parent->SetTerminalCount(m_id, false);
	}

	void DMAChannel::DMAOperation(BYTE& value)
	{
		// Page register is not shifted, bit 0 is cleared and ignored
		emul::ADDRESS addr = (m_page << 16) + (m_address << m_addressShift ? 1 : 0);

		if (m_terminalCount)
		{
			return;
		}

		switch (m_operation)
		{
		case OPERATION::READ:
			value = m_memory.Read8(addr);
			LogPrintf(LOG_DEBUG, "DMA Read, value=%02X @ Address %04x", value, addr);
			break;
		case OPERATION::WRITE:
			LogPrintf(LOG_DEBUG, "DMA Write, value=%02X @ Address %04x", value, addr);
			m_memory.Write8(addr, value);
			break;
		case OPERATION::VERIFY:
			LogPrintf(LOG_DEBUG, "DMA Verify, value=%02X @ Address %04x", value, addr);
			break;
		default:
			throw std::exception("DMAOperation: Operation not supported");
		}

		Tick();
	}

	Device8237::Device8237(const char* id, WORD baseAddress, emul::Memory& memory) :
		Logger(id),
		m_channel0(this, memory, 0, std::string(id) + ".0"),
		m_channel1(this, memory, 1, std::string(id) + ".1"),
		m_channel2(this, memory, 2, std::string(id) + ".2"),
		m_channel3(this, memory, 3, std::string(id) + ".3"),
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

	Device8237::Device8237(WORD baseAddress, emul::Memory& memory) :
		Device8237("dma", baseAddress, memory)
	{
	}

	void Device8237::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);

		m_channel0.EnableLog(minSev);
		m_channel1.EnableLog(minSev);
		m_channel2.EnableLog(minSev);
		m_channel3.EnableLog(minSev);
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
		DMARequests[3] = false;
	}

	void Device8237::Init(BYTE addressShift)
	{
		// Registers port 0..7
		m_channel0.Init(addressShift);
		m_channel1.Init(addressShift);
		m_channel2.Init(addressShift);
		m_channel3.Init(addressShift);

		// Ports 8..16, control, etc.

		// base+8: Command Register (write), Status Register (read)
		Connect(GetBaseAddress() + (0x8 << addressShift), static_cast<PortConnector::INFunction>(&Device8237::ReadStatus));
		Connect(GetBaseAddress() + (0x8 << addressShift), static_cast<PortConnector::OUTFunction>(&Device8237::WriteCommand));

		// base+9: Request (write)
		Connect(GetBaseAddress() + (0x9 << addressShift), static_cast<PortConnector::OUTFunction>(&Device8237::WriteRequest));

		// base+A: Single Mask Bit (write)
		Connect(GetBaseAddress() + (0xA << addressShift), static_cast<PortConnector::OUTFunction>(&Device8237::WriteSingleMaskBit));

		// base+B: Mode (write)
		Connect(GetBaseAddress() + (0xB << addressShift), static_cast<PortConnector::OUTFunction>(&Device8237::WriteMode));

		// base+C: Clear Byte Flip-flop (write)
		Connect(GetBaseAddress() + (0xC << addressShift), static_cast<PortConnector::OUTFunction>(&Device8237::ClearFlipFlop));

		// base+D: Master Clear (write), Temporary Register (read)
		Connect(GetBaseAddress() + (0xD << addressShift), static_cast<PortConnector::INFunction>(&Device8237::ReadTemp));
		Connect(GetBaseAddress() + (0xE << addressShift), static_cast<PortConnector::OUTFunction>(&Device8237::MasterClear));

		// base+E: All Mask Bits (write)
		Connect(GetBaseAddress() + 0xF, static_cast<PortConnector::OUTFunction>(&Device8237::WriteAllMaskBits));

		// TODO: Move this out of here
		// Page Registers
		Connect(0x87, static_cast<PortConnector::OUTFunction>(&Device8237::WriteChannel0Page));
		Connect(0x83, static_cast<PortConnector::OUTFunction>(&Device8237::WriteChannel1Page));
		Connect(0x81, static_cast<PortConnector::OUTFunction>(&Device8237::WriteChannel2Page));
		Connect(0x82, static_cast<PortConnector::OUTFunction>(&Device8237::WriteChannel3Page));
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
		GetChannel(value & 3).SetMode(value);
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
		if (channel > 3) throw std::exception("invalid dma channel");
		DMARequests[channel & 3] = state;
	}
	bool Device8237::DMAAcknowledged(BYTE channel)
	{
		if (channel > 3) throw std::exception("invalid dma channel");
		// TODO
		bool ack = DMARequests[channel & 3];
		DMARequests[channel & 3] = false;
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

	void Device8237::SetTerminalCount(BYTE channel, bool tc)
	{
		channel &= 3;
		m_statusReg &= ~(1 << channel);

		if (tc)
		{
			m_statusReg |= (1 << channel);
		}
	}

	bool Device8237::GetTerminalCount(BYTE channel)
	{
		channel &= 3;
		return m_statusReg & (1 << channel);
	}

	DMAChannel& Device8237::GetChannel(BYTE channel)
	{
		switch (channel & 3)
		{
		case 0: return m_channel0;
		case 1: return m_channel1;
		case 2: return m_channel2;
		case 3: return m_channel3;
		default:
			throw std::exception("not possible");
		}
	}

	void Device8237::WriteChannel0Page(BYTE value)
	{
		LogPrintf(LOG_INFO, "Set Channel 0 Page Register, value=%02X", value);
		m_channel0.SetPage(value);
	}
	void Device8237::WriteChannel1Page(BYTE value)
	{
		LogPrintf(LOG_INFO, "Set Channel 1 Page Register, value=%02X", value);
		m_channel1.SetPage(value);
	}
	void Device8237::WriteChannel2Page(BYTE value)
	{
		LogPrintf(LOG_INFO, "Set Channel 2 Page Register, value=%02X", value);
		m_channel2.SetPage(value);
	}
	void Device8237::WriteChannel3Page(BYTE value)
	{
		LogPrintf(LOG_INFO, "Set Channel 3 Page Register, value=%02X", value);
		m_channel3.SetPage(value);
	}
}
