#include "Device8259.h"
#include <assert.h>

using emul::SetBit;
using emul::LowestSetBit;
using emul::IsPowerOf2;
using emul::LogBase2;

namespace pic
{
	Device8259::Device8259(WORD baseAddress) :
		Logger("pic"),
		m_baseAddress(baseAddress)
	{
		Reset();
	}

	void Device8259::Reset()
	{
		m_state = STATE::UNINITIALIZED;
	}

	void Device8259::InterruptRequest(BYTE interrupt)
	{
		LogPrintf(LOG_INFO, "InterruptRequest: int=%d", interrupt);
		assert(interrupt < 8);
		BYTE intBit = 1 << interrupt;

		// Mask bits: 0 = Enabled, 1 = Interrupt Masked (inhibited)
		intBit &= ~m_interruptMaskRegister;

		if (intBit)
		{
			m_interruptRequestRegister |= intBit;
		}
		else
		{
			LogPrintf(LOG_INFO, "InterruptRequest: Interrupt is masked");
		}
	}

	void Device8259::Init()
	{
		Connect(m_baseAddress, static_cast<PortConnector::INFunction>(&Device8259::Read0));
		Connect(m_baseAddress, static_cast<PortConnector::OUTFunction>(&Device8259::Write0));

		Connect(m_baseAddress+1, static_cast<PortConnector::INFunction>(&Device8259::Read1));
		Connect(m_baseAddress+1, static_cast<PortConnector::OUTFunction>(&Device8259::Write1));
	}

	BYTE Device8259::Read0()
	{
		LogPrintf(LOG_DEBUG, "Read0, value=%02X", *m_reg0);

		return *m_reg0;
	}
	void Device8259::Write0(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write0, value=%02X", value);

		if (value & 16)
		{
			LogPrintf(LOG_INFO, "ICW1: Begin initialization sequence");
			m_interruptMaskRegister = 0;
			m_interruptRequestRegister = 0;
			m_reg0 = &m_interruptRequestRegister;

			m_init.icw4Needed = (value & 1);
			LogPrintf(LOG_INFO, "ICW1: ICW4 needed: %d", m_init.icw4Needed);
			if (!m_init.icw4Needed)
			{
				throw std::exception("ICW1: icw4Needed == false not supported");
			}

			m_init.single = (value & 2);
			LogPrintf(LOG_INFO, "ICW1: SINGLE mode: %d", m_init.single);
			if (!m_init.single)
			{
				throw std::exception("ICW1: cascade mode not supported");
			}

			m_init.levelTriggered = (value & 8);
			LogPrintf(LOG_INFO, "ICW1: %s trigger mode", m_init.levelTriggered ? "LEVEL" : "EDGE");
			if (m_init.levelTriggered)
			{
				throw std::exception("ICW1: LEVEL trigger mode supported");
			}

			m_state = STATE::ICW2;
		} 
		else if (m_state == STATE::READY)
		{
			switch (value & 0b00011000)
			{
			case 0: OCW2(value); break;
			case 0b00001000: OCW3(value); break;
			default:
				LogPrintf(LOG_ERROR, "Write0: Invalid value %d", value);
				throw std::exception("Write0: Invalid value ");
			}
		}
		else
		{
			LogPrintf(LOG_ERROR, "Write0: Invalid state %d", m_state);
			throw std::exception("Write0: Invalid state");
		}

	}

	void Device8259::OCW2(BYTE value)
	{
		switch (value >> 5)
		{
		case 0b001:
			LogPrintf(LOG_INFO, "OCW2: Non-specific EOI");
			EOI();
			break;

		case 0b011:
			LogPrintf(LOG_ERROR, "OCW2: Specific EOI, not implemented");
			break;

		case 0b101:
		case 0b100:
		case 0b000:
		case 0b111:
			LogPrintf(LOG_ERROR, "OCW2: Rotate, not implemented");
			break;

		case 0b110:
			LogPrintf(LOG_ERROR, "OCW2: Set Priority, not implemented");
			break;

		case 0b010:
			LogPrintf(LOG_INFO, "OCW2: No operation");
			break;
		}
	}
	void Device8259::OCW3(BYTE value)
	{
		switch (value & 3)
		{
		case 0b00:
		case 0b01:
			// No action
			break;
		case 0b10:
			LogPrintf(LOG_INFO, "OCW3: Read Interrupt Request (IR) Register");
			m_reg0 = &m_interruptRequestRegister;
			break;
		case 0b11:
			LogPrintf(LOG_INFO, "OCW3: Read In Service (IS) Register");
			m_reg0 = &m_inServiceRegister;
			break;
		}
	}

	BYTE Device8259::Read1()
	{
		LogPrintf(LOG_DEBUG, "Read1: value=%02X", m_interruptMaskRegister);

		switch (m_state)
		{
		case STATE::READY:
			LogPrintf(LOG_INFO, "OCW1: Read Mask: %02Xh", m_interruptMaskRegister);
			return m_interruptMaskRegister;
		default:
			LogPrintf(LOG_ERROR, "Read1: Invalid state %d", m_state);
			throw std::exception("Read1: Invalid state");
		}

		return 0xFF;
	}

	void Device8259::Write1(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write1: value=%02X", value);

		switch (m_state)
		{
		case STATE::ICW2:
			// Assume 8086 mode, ICW2 contains upper 4 bits of 
			// interrupt # to send to CPU
			m_init.interruptBase = value & 0b11111000;
			LogPrintf(LOG_INFO, "ICW2: Interrupt Base: %02Xh", m_init.interruptBase);

			m_state = m_init.single ? (m_init.icw4Needed ? STATE::ICW4 : STATE::READY) : STATE::ICW3;
			break;
		case STATE::ICW3:
			throw std::exception("ICW3: not supported");
			break;
		case STATE::ICW4:
			m_init.cpu8086 = (value & 1);
			LogPrintf(LOG_INFO, "ICW4: CPU Mode: %s", m_init.cpu8086 ? "8086" : "8080");
			if (!m_init.cpu8086)
			{
				throw std::exception("ICW4: cpu8086 == false not supported");
			}

			m_init.autoEOI = (value & 2);
			LogPrintf(LOG_INFO, "ICW4: %s EOI", m_init.autoEOI ? "AUTO" : "NORMAL");
			if (m_init.autoEOI)
			{
				throw std::exception("ICW4: autoEOI == true not supported");
			}

			// D2 = master slave, not used

			m_init.buffered = (value & 8);
			LogPrintf(LOG_INFO, "ICW4: Buffered mode: %d", m_init.buffered);
			if (!m_init.buffered)
			{
				throw std::exception("ICW4: Non buffered mode not supported");
			}

			m_init.sfnm = (value & 16);
			LogPrintf(LOG_INFO, "ICW4: Special Fully Nested mode: %d", m_init.sfnm);
			if (m_init.sfnm)
			{
				throw std::exception("ICW4: Special Fully Nested mode not supported");
			}
			m_state = STATE::READY;
			break;

		case STATE::READY:
			LogPrintf(LOG_INFO, "OCW1: Set Mask: %02Xh", value);
			m_interruptMaskRegister = value;
			break;
		default:
			LogPrintf(LOG_ERROR, "Write1: Invalid state %d", m_state);
			throw std::exception("Write1: Invalid state");
		}

	}

	void Device8259::Tick()
	{
	}

	// TODO: simplification, doesn't handle multiple interrupts & priorities correctly
	void Device8259::InterruptAcknowledge()
	{
		// Lowest set bit (higher priority by default)
		BYTE intBit = LowestSetBit(m_interruptRequestRegister);

		m_inServiceRegister |= intBit; // Set bit in ISR
		m_interruptRequestRegister &= ~intBit; // Clear bit in IRR
	}

	// TODO: Only handles one active interrupt at a time
	BYTE Device8259::GetPendingInterrupt() const
	{
		assert(IsPowerOf2(m_inServiceRegister));
		return LogBase2(m_inServiceRegister) | m_init.interruptBase;
	}

	// TODO: Only handles one active interrupt at a time
	void Device8259::EOI()
	{
		m_inServiceRegister = 0;
	}

}
