#include "stdafx.h"

#include "Device8259.h"

using emul::GetBit;
using emul::SetBit;
using emul::LowestSetBit;
using emul::IsPowerOf2;
using emul::LogBase2;

namespace pic
{
	Device8259::Device8259(const char* id, WORD baseAddress, bool isPrimary) :
		Logger(id),
		m_baseAddress(baseAddress)
	{
		m_cascade.primary = isPrimary;
	}

	Device8259::Device8259(WORD baseAddress, bool isPrimary) : Device8259("pic", baseAddress, isPrimary)
	{
	}

	void Device8259::INIT::Reset()
	{
		memset(this, 0, sizeof(INIT));
	}

	void Device8259::Cascade::Reset()
	{ 
		this->secondaryConnected.fill(false);
		this->primaryIRQ = 0;
	}

	void Device8259::Reset()
	{
		m_state = STATE::UNINITIALIZED;

		m_lastInterruptRequestRegister = 0xFF;
		m_interruptRequestRegister = 0;
		m_inServiceRegister = 0;
		m_interruptMaskRegister = 0;
		m_reg0 = &m_inServiceRegister;

		m_init.Reset();
		m_cascade.Reset();
	}

	void Device8259::AttachSecondaryDevice(BYTE irq, Device8259* secondary)
	{
		if (irq > 7)
		{
			throw std::exception("AttachSecondaryDevice: irq > 7 not supported");
		}
		LogPrintf(LOG_INFO, "Setting secondary device for IRQ%d", irq);
		m_cascade.secondaryDevices[irq] = secondary;
		secondary->SetPrimary(this);
	}

	void Device8259::InterruptRequest(BYTE interrupt, bool value)
	{
		if (m_state != STATE::READY)
		{
			return;
		}

		assert(interrupt < 8);

		// Edge triggered mode only
		if (!GetBit(m_lastInterruptRequestRegister, interrupt))
		{
			SetBit(m_interruptRequestRegister, interrupt, value);
			SetBit(m_lastInterruptRequestRegister, interrupt, value);

			if (value)
			{
				LogPrintf(LOG_DEBUG, "InterruptRequest: int=%d got edge", interrupt);
			}
		}
		else if (!value)
		{
			SetBit(m_interruptRequestRegister, interrupt, false);
			SetBit(m_lastInterruptRequestRegister, interrupt, false);
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
		LogPrintf(LOG_INFO, "Read0, value=%02X", *m_reg0);

		return *m_reg0;
	}
	void Device8259::Write0(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write0, value=%02X", value);

		if (value & 16)
		{
			LogPrintf(LOG_INFO, "ICW1: Begin initialization sequence");
			m_interruptMaskRegister = 0;
			m_inServiceRegister = 0;

			// Reset IRR edge detection
			m_interruptRequestRegister = 0;
			m_lastInterruptRequestRegister = 0xFF;

			m_reg0 = &m_interruptRequestRegister;

			m_init.icw4Needed = (value & 1);
			LogPrintf(LOG_INFO, "ICW1: ICW4 needed: %d", m_init.icw4Needed);
			if (!m_init.icw4Needed)
			{
				throw std::exception("ICW1: icw4Needed == false not supported");
			}

			m_init.single = (value & 2);
			LogPrintf(LOG_INFO, "ICW1: SINGLE mode: %d", m_init.single);

			m_init.levelTriggered = (value & 8);
			LogPrintf(LOG_INFO, "ICW1: %s trigger mode", m_init.levelTriggered ? "LEVEL" : "EDGE");
			if (m_init.levelTriggered)
			{
				throw std::exception("ICW1: LEVEL trigger not mode supported");
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
			NonSpecificEOI();
			break;

		case 0b011:
			LogPrintf(LOG_INFO, "OCW2: Specific EOI [%d]", (value & 0b111));
			SpecificEOI(value);
			break;

		case 0b101:
		case 0b100:
		case 0b000:
		case 0b111:
			throw std::exception("OCW2: Rotate, not implemented");
			break;

		case 0b110:
			throw std::exception("OCW2: Set Priority, not implemented");
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
			if (m_init.single)
			{
				throw std::exception("ICW3: Single mode, invalid state");
			}
			if (m_cascade.primary)
			{
				LogPrintf(LOG_INFO, "ICW3: Primary device, setting connected secondary devices:");
				for (int i = 0; i < 7; ++i)
				{
					bool connected = GetBit(value, i);
					LogPrintf(LOG_INFO, "ICW3: - Secondary[%d] : %s", i, connected ? "YES" : "NO");
					m_cascade.secondaryConnected[i] = connected;
					if (connected && !m_cascade.secondaryDevices[i])
					{
						LogPrintf(LOG_WARNING, "ICW3: Warning: secondary device is not connected");
					}
				}
			}
			else
			{
				m_cascade.primaryIRQ = value & 7;
				LogPrintf(LOG_INFO, "ICW3: Secondary device, set primary IRQ=[%d]", m_cascade.primaryIRQ);
				if (!m_cascade.primaryDevice)
				{
					LogPrintf(LOG_WARNING, "ICW3: Warning: primary device is not connected");
				}
			}
			m_state = m_init.icw4Needed ? STATE::ICW4 : STATE::READY;
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

			// TODO: Buffered mode not implemented, not needed until we cascade timers
			m_init.buffered = (value & 8);
			LogPrintf(LOG_INFO, "ICW4: Buffered mode: %d", m_init.buffered);

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

	bool Device8259::InterruptPending() const
	{ 
		BYTE requests = m_interruptRequestRegister & (~m_interruptMaskRegister);
		if (requests)
		{
			if (!m_inServiceRegister)
			{
				return true;
			}

			// If already servicing an interrupt, allow if there is a higher priority interrupt pending
			BYTE pending = LowestSetBit(requests);
			BYTE servicing = LowestSetBit(m_inServiceRegister);

			return pending < servicing;
		}
		else
		{
			return false;
		}
	}

	void Device8259::InterruptAcknowledge()
	{
		BYTE requests = m_interruptRequestRegister & (~m_interruptMaskRegister);
		if (!requests)
		{
			requests = 0x80; // Fallback to IRQ7
		}

		// Lowest set bit (higher priority by default)
		BYTE intBit = LowestSetBit(requests);
		LogPrintf(LOG_DEBUG, "Acknowledge, IRQ(bit): %02xh", intBit);

		m_inServiceRegister |= intBit; // Set bit in ISR
		m_interruptRequestRegister &= ~intBit; // Clear bit in IRR
	}

	BYTE Device8259::GetPendingInterrupt() const
	{
		BYTE irq = LogBase2(LowestSetBit(m_inServiceRegister));
		LogPrintf(LOG_DEBUG, "Pending IRQ: %d", irq);
		return irq | m_init.interruptBase;
	}

	void Device8259::NonSpecificEOI()
	{
		BYTE intBit = LowestSetBit(m_inServiceRegister);
		LogPrintf(LOG_DEBUG, "EOI: ISR Before: %02xh", m_inServiceRegister);
		m_inServiceRegister &= ~intBit; // Clear bit in IRR
		LogPrintf(LOG_DEBUG, "EOI: ISR After: %02xh", m_inServiceRegister);
	}

	void Device8259::SpecificEOI(BYTE level)
	{
		LogPrintf(LOG_DEBUG, "EOI: ISR Before: %02xh", m_inServiceRegister);
		SetBit(m_inServiceRegister, (level & 0b111), false);
		LogPrintf(LOG_DEBUG, "EOI: ISR After: %02xh", m_inServiceRegister);
	}

	void Device8259::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;

		to["state"] = m_state;

		json init;
		init["icw4Needed"] = m_init.icw4Needed;
		init["single"] = m_init.single;
		init["levelTriggered"] = m_init.levelTriggered;
		init["interruptBase"] = m_init.interruptBase;
		init["cpu8086"] = m_init.cpu8086;
		init["autoEOI"] = m_init.autoEOI;
		init["buffered"] = m_init.buffered;
		init["sfnm"] = m_init.sfnm;
		to["init"] = init;

		json cascade;
		cascade["primary"] = m_cascade.primary;
		cascade["secondaryConnected"] = m_cascade.secondaryConnected;
		cascade["primaryIRQ"] = m_cascade.primaryIRQ;
		to["cascade"] = cascade;

		to["irr0"] = m_lastInterruptRequestRegister;
		to["irr"] = m_interruptRequestRegister;
		to["isr"] = m_inServiceRegister;
		to["imr"] = m_interruptMaskRegister;
		to["reg0"] = (m_reg0 == &m_inServiceRegister) ? "isr" : "irr";
	}
	void Device8259::Deserialize(const json& from)
	{
		WORD baseAddress = from["baseAddress"];
		if (baseAddress != m_baseAddress)
		{
			throw emul::SerializableException("Device8259: Incompatible baseAddress");
		}

		m_state = from["state"];

		const json& init = from["init"];
		m_init.icw4Needed = init["icw4Needed"];
		m_init.single = init["single"];
		m_init.levelTriggered = init["levelTriggered"];
		m_init.interruptBase = init["interruptBase"];
		m_init.cpu8086 = init["cpu8086"];
		m_init.autoEOI = init["autoEOI"];
		m_init.buffered = init["buffered"];
		m_init.sfnm = init["sfnm"];

		const json& cascade = from["cascade"];
		m_cascade.primary = cascade["primary"];
		m_cascade.secondaryConnected = cascade["secondaryConnected"];
		m_cascade.primaryIRQ = cascade["primaryIRQ"];

		m_lastInterruptRequestRegister = from["irr0"];
		m_interruptRequestRegister = from["irr"];
		m_inServiceRegister = from["isr"];
		m_interruptMaskRegister = from["imr"];

		if (from["reg0"] == "isr")
		{
			m_reg0 = &m_inServiceRegister;
		}
		else
		{
			m_reg0 = &m_interruptRequestRegister;
		}
	}
}
