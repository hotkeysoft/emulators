#include "stdafx.h"

#include "Device8530.h"

using hscommon::EdgeDetectLatch;
using Mask = emul::BitMaskB;
using emul::BYTE;
using emul::GetBit;

namespace scc
{
	SCCChannel::SCCChannel(std::string id) : Logger(id.c_str())
	{
	}

	void SCCChannel::Init(Device8530* parent)
	{
		assert(parent);
		m_parent = parent;
	}

	void SCCChannel::Reset(bool hard)
	{
		LogPrintf(LOG_INFO, "Reset (%s)", hard ? "hardware" : "channel");

		Mask("00000000").Apply(m_regs.WR0);
		Mask("00x00x00").Apply(m_regs.WR1);
		Mask("xxxxxxx0").Apply(m_regs.WR3);
		Mask("xxxxx1xx").Apply(m_regs.WR4);
		//Mask("0xx0000x").Apply(WR5);
		//Mask("xxxxxxxx").Apply(WR6);
		//Mask("xxxxxxxx").Apply(WR7);
		//Mask(hard ? "00000000" : "0xx00000").Apply(WR10);
		//Mask(hard ? "00001000" : "xxxxxxxx").Apply(WR11);
		//Mask("xxxxxxxx").Apply(WR12);
		//Mask("xxxxxxxx").Apply(WR13);
		//Mask(hard ? "xx100000" : "xx1000xx").Apply(WR14);
		Mask("11111000").Apply(m_regs.WR15);

		Mask("01xxx100").Apply(m_regs.RR0);
		//Mask("00000110").Apply(RR1);
		//Mask("00000000").Apply(RR3);
		//Mask("00000000").Apply(RR10);
	}

	BYTE SCCChannel::ReadData() 
	{ 
		LogPrintf(LOG_INFO, "ReadData");
		return 0xFF; 
	}
	void SCCChannel::WriteData(BYTE value) 
	{
		LogPrintf(LOG_INFO, "WriteData, value=%02X", value);
	}

	BYTE SCCChannel::ReadControl() 
	{ 
		LogPrintf(LOG_TRACE, "ReadControl");

		BYTE value = 0xFF;
		switch (m_parent->GetCurrRegister())
		{
		case 0:  value = RR0(); break;
		case 1:  value = RR1(); break;
		case 3:  value = RR3(); break;
		case 10: value = RR10(); break;
		default:
			LogPrintf(LOG_WARNING, "ReadControl: Invalid read register %d", m_parent->GetCurrRegister());
		}

		m_parent->SetCurrRegister(0);

		return value;
	}
	void SCCChannel::WriteControl(BYTE value) 
	{
		LogPrintf(LOG_TRACE, "WriteControl, value=%02X", value);

		bool resetCurrRegister = true;
		switch (m_parent->GetCurrRegister())
		{
		case 0:  WR0(value); resetCurrRegister = false; break;
		case 1:  WR1(value); break;
		case 2:  m_parent->WR2(value); break;
		case 3:  WR3(value); break;
		case 4:  WR4(value); break;
		case 5:  WR5(value); break;
		case 6:  WR6(value); break;
		case 7:  WR7(value); break;
		case 9:  m_parent->WR9(value); break;
		case 10: WR10(value); break;
		case 11: WR11(value); break;
		case 12: WR12(value); break;
		case 13: WR13(value); break;
		case 14: WR14(value); break;
		case 15: WR15(value); break;
		default:
			LogPrintf(LOG_WARNING, "WriteControl: Invalid write register %d", m_parent->GetCurrRegister());
		}

		if (resetCurrRegister)
		{
			m_parent->SetCurrRegister(0);
		}
	}

	// Command Register
	void SCCChannel::WR0(BYTE value) 
	{
		m_regs.WR0 = value;
		LogPrintf(LOG_DEBUG, "WR0(%02X)", value);

		int selRegister = value & 7;

		// Command
		switch ((value >> 3) & 7)
		{
		case 0: // NULL
			break;
		case 1: // Point High
			LogPrintf(LOG_DEBUG, "WR0: Point High");
			selRegister += 8;
			break;
		case 2: // Reset ext/status interrupts
			LogPrintf(LOG_WARNING, "WR0: Reset ext/status interrupts (not implemented)");
			break;
		case 3: // Sent abort (SDLC)
			LogPrintf(LOG_WARNING, "WR0: Sent abort (SDLC) (not implemented)");
			break;
		case 4: // Enable INT on next Rx character
			LogPrintf(LOG_WARNING, "WR0: Enable INT on next Rx character (not implemented)");
			break;
		case 5: // Reset TxINT pending
			LogPrintf(LOG_WARNING, "WR0: Reset TxINT pending (not implemented)");
			break;
		case 6: // Error reset
			LogPrintf(LOG_WARNING, "WR0: Error reset (not implemented)");
			break;
		case 7: // Reset highest IUS
			LogPrintf(LOG_WARNING, "WR0: Reset highest IUS (not implemented)");
			break;
		default:
			NODEFAULT;
		}

		// CRC Reset
		switch ((value >> 6) & 3)
		{
		case 0:
			break;
		case 1:
			LogPrintf(LOG_WARNING, "WR0: Reset Rx CRC Checker (not implemented)");
			break;
		case 2:
			LogPrintf(LOG_WARNING, "WR0: Reset Tx CRC Generator (not implemented)");
			break;
		case 3:
			LogPrintf(LOG_WARNING, "WR0: Reset Tx Underrun/EOM Latch (not implemented)");
			break;
		}

		LogPrintf(LOG_DEBUG, "WR0: SetCurrRegister %d", selRegister);
		m_parent->SetCurrRegister(selRegister);
	}

	// Transmit/Receive Interrupt and Data Transfer Mode Definition
	void SCCChannel::WR1(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WR1(%02X)", value);

		LogPrintf(LOG_INFO, "WR1: Tx/Rx Interrupt and Data Transfer Mode");

		LogPrintf(LOG_INFO, " Wait/DMA Request Enable     : %d", GetBit(value, 7));
		LogPrintf(LOG_INFO, " Wait/DMA Request Function   : %d", GetBit(value, 6));
		LogPrintf(LOG_INFO, " Wait/DMA Request on Rx/Tx   : %d", GetBit(value, 5));
		switch ((value >> 3) & 3)
		{
		case 0: LogPrintf(LOG_INFO, " Rx Interrupt                : Disable"); break;
		case 1: LogPrintf(LOG_INFO, " Rx Interrupt                : First char | Special Condition"); break;
		case 2: LogPrintf(LOG_INFO, " Rx Interrupt                : All chars | Special Condition"); break;
		case 3: LogPrintf(LOG_INFO, " Rx Interrupt                : Special Condition only"); break;
		default:
			NODEFAULT;
		}
		LogPrintf(LOG_INFO, " Parity is special condition : %d", GetBit(value, 2));
		LogPrintf(LOG_INFO, " Tx int enable               : %d", GetBit(value, 1));
		LogPrintf(LOG_INFO, " External int enable         : %d", GetBit(value, 0));
	}

	// Receiver Bits/Character
	void SCCChannel::WR3(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WR3(%02X)", value);
		m_regs.WR3 = value;

		LogPrintf(LOG_INFO, "WR3: Receiver Bits/Character");
		switch ((value >> 6) & 3)
		{
		case 0: LogPrintf(LOG_INFO, " Bits/Character : 5"); break;
		case 1: LogPrintf(LOG_INFO, " Bits/Character : 7"); break;
		case 2: LogPrintf(LOG_INFO, " Bits/Character : 6"); break;
		case 3: LogPrintf(LOG_INFO, " Bits/Character : 8"); break;
		default:
			NODEFAULT;
		}

		LogPrintf(LOG_INFO, " Auto Enables                 : %d", GetBit(value, 5));
		LogPrintf(LOG_INFO, " Enter Hunt Mode              : %d", GetBit(value, 4));
		LogPrintf(LOG_INFO, " Rx CRC Enable                : %d", GetBit(value, 3));
		LogPrintf(LOG_INFO, " Address Search Mode (SDLC)   : %d", GetBit(value, 2));
		LogPrintf(LOG_INFO, " Sync Character Load Inhibit  : %d", GetBit(value, 1));
		LogPrintf(LOG_INFO, " Rx Enable                    : %d", GetBit(value, 0));
	}

	// Transmit/Receiver misc parameters and modes
	void SCCChannel::WR4(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WR4(%02X)", value);
		m_regs.WR4 = value;

		LogPrintf(LOG_INFO, "WR4: Transmit/Receiver misc parameters and modes");
		switch ((value >> 6) & 3)
		{
		case 0: LogPrintf(LOG_INFO, " Clock Mode : x1"); break;
		case 1: LogPrintf(LOG_INFO, " Clock Mode : x16"); break;
		case 2: LogPrintf(LOG_INFO, " Clock Mode : x32"); break;
		case 3: LogPrintf(LOG_INFO, " Clock Mode : x64"); break;
		default:
			NODEFAULT;
		}

		switch ((value >> 2) & 3)
		{
		case 0: // Sync modes
		{
			switch ((value >> 4) & 3)
			{
			case 0: LogPrintf(LOG_INFO, " Sync Mode  : 8 bit sync character"); break;
			case 1: LogPrintf(LOG_INFO, " Sync Mode  : 16 bit sync character"); break;
			case 2: LogPrintf(LOG_INFO, " Sync Mode  : SDLC"); break;
			case 3: LogPrintf(LOG_INFO, " Sync Mode  : External"); break;
			default:
				NODEFAULT;
			}
		}
		case 1: LogPrintf(LOG_INFO, " Stop Mode  : 2 stop bit"); break;
		case 2: LogPrintf(LOG_INFO, " Stop Mode  : 1.5 stop bits"); break;
		case 3: LogPrintf(LOG_INFO, " Stop Mode  : 2 stop bits"); break;
		default:
			NODEFAULT;
		}

		LogPrintf(LOG_INFO, " Parity     : %s", 
			GetBit(value, 0) ? (GetBit(value, 1) ? "Even" : "Odd") : "None");
	}
	void SCCChannel::WR5(BYTE value)
	{
		LogPrintf(LOG_INFO, "WR5(%02X)", value);
	}
	void SCCChannel::WR6(BYTE value)
	{
		LogPrintf(LOG_INFO, "WR6(%02X)", value);
	}
	void SCCChannel::WR7(BYTE value)
	{
		LogPrintf(LOG_INFO, "WR7(%02X)", value);
	}
	void SCCChannel::WR10(BYTE value)
	{
		LogPrintf(LOG_INFO, "WR10(%02X)", value);
	}
	void SCCChannel::WR11(BYTE value)
	{
		LogPrintf(LOG_INFO, "WR11(%02X)", value);
	}
	void SCCChannel::WR12(BYTE value)
	{
		LogPrintf(LOG_INFO, "WR12(%02X)", value);
	}
	void SCCChannel::WR13(BYTE value)
	{
		LogPrintf(LOG_INFO, "WR13(%02X)", value);
	}
	void SCCChannel::WR14(BYTE value)
	{
		LogPrintf(LOG_INFO, "WR14(%02X)", value);
	}
	void SCCChannel::WR15(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WR15(%02X)", value);
		m_regs.WR15 = value;

		LogPrintf(LOG_INFO, "WR15: External/Status Interrupt Control");

		LogPrintf(LOG_INFO, " IE - Break/Abort     : %d", GetBit(value, 7));
		LogPrintf(LOG_INFO, " IE - Tx Underrun/EOM : %d", GetBit(value, 6));
		LogPrintf(LOG_INFO, " IE - CTS             : %d", GetBit(value, 5));
		LogPrintf(LOG_INFO, " IE - Sync/Hunt       : %d", GetBit(value, 4));
		LogPrintf(LOG_INFO, " IE - DCD             : %d", GetBit(value, 3));
		LogPrintf(LOG_INFO, " IE - Zero Count      : %d", GetBit(value, 1));
	}

	// Transmit/Receive buffer Status and External Status
	BYTE SCCChannel::RR0()
	{
		LogPrintf(LOG_DEBUG, "RR0");
		return m_regs.RR0;
	}
	BYTE SCCChannel::RR1()
	{
		LogPrintf(LOG_INFO, "RR1");
		return 0xFF;
	}
	BYTE SCCChannel::RR3()
	{
		LogPrintf(LOG_INFO, "RR3");
		return 0xFF;
	}
	BYTE SCCChannel::RR10()
	{
		LogPrintf(LOG_INFO, "RR10");
		return 0xFF;
	}

	void SCCChannel::Serialize(json& to)
	{
	}

	void SCCChannel::Deserialize(const json& from)
	{
	}

	Device8530::Device8530(std::string id) :
		Logger(id.c_str()),
		m_channelA(id + ".A"),
		m_channelB(id + ".B")
	{
	}

	void Device8530::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		m_channelA.EnableLog(minSev);
		m_channelB.EnableLog(minSev);
	}

	void Device8530::Init()
	{
		m_channelA.Init(this);
		m_channelB.Init(this);
	}

	void Device8530::Reset(bool hard)
	{
		LogPrintf(LOG_INFO, "Reset (%s)", hard ? "hard" : "soft");

		m_channelA.Reset(true);
		m_channelB.Reset(true);
		m_currRegister = 0;

		Mask(hard ? "110000xx" : "xx0xxxxx").Apply(m_regs.WR9);
	}

	void Device8530::WR2(BYTE value)
	{
		m_regs.WR2 = value;
		LogPrintf(LOG_INFO, "WR2: Interrupt Vector [%02X]", value);
	}

	// Master Interrupt Control
	void Device8530::WR9(BYTE value)
	{
		m_regs.WR9 = value;

		LogPrintf(LOG_TRACE, "WR9(%02X)", value);

		LogPrintf(LOG_INFO, "WR9: Master Interrupt Control");

		LogPrintf(LOG_INFO, " Master Interrupt Enable : %d", GetBit(value, 3));
		LogPrintf(LOG_INFO, " Disable Lower Chain     : %d", GetBit(value, 2));
		LogPrintf(LOG_INFO, " No Vector               : %d", GetBit(value, 1));
		LogPrintf(LOG_INFO, " Vector Status High/Low  : %c", GetBit(value, 4) ? 'H' : 'L');
		LogPrintf(LOG_INFO, " Vector Includes Status  : %d", GetBit(value, 0));

		switch ((value >> 6) & 3)
		{
		case 0: // No reset
			break;
		case 1: // Channel B Reset
			m_channelB.Reset(false);
			break;
		case 2: // Channel A Reset
			m_channelA.Reset(false);
			break;
		case 3: // Force Hardware Reset
			Reset(false);
			break;
		}
	}

	void Device8530::Serialize(json& to)
	{
		m_channelA.Serialize(to["channelA"]);
		m_channelB.Serialize(to["channelB"]);
	}

	void Device8530::Deserialize(const json& from)
	{
		m_channelA.Deserialize(from["channelA"]);
		m_channelB.Deserialize(from["channelB"]);
	}
}
