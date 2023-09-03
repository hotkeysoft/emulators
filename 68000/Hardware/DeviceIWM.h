#pragma once

#include <CPU/Memory.h>
#include <Logger.h>

namespace floppy::woz
{
	class DeviceIWM : public Logger
	{
	public:
		DeviceIWM() : Logger("IWM") { Reset(); }
		virtual ~DeviceIWM() {}

		DeviceIWM(const DeviceIWM&) = delete;
		DeviceIWM& operator=(const DeviceIWM&) = delete;
		DeviceIWM(DeviceIWM&&) = delete;
		DeviceIWM& operator=(DeviceIWM&&) = delete;

		void Reset();

		void SetStateRegister(BYTE a3a2a1a0);

		void SetSel(bool sel) { LogPrintf(LOG_INFO, "Set SEL: %d", sel); m_sel = sel; }

		void Write(BYTE value);
		BYTE Read();

	protected:
		bool IsMotorOn() const { return emul::GetBit(m_stateRegister, 4); }
		bool GetDriveSel() const { return emul::GetBit(m_stateRegister, 5); }
		int GetL7L6() const { return m_stateRegister >> 6; }

		BYTE ReadStatus();
		BYTE ReadData();
		BYTE ReadWriteHandshake();

		void WriteRegister();

		void WriteMode(BYTE value);
		void WriteData(BYTE value);

		enum class State
		{
			NONE,
			READ_DATA,
			READ_STATUS,
			READ_WRITE_HANDSHAKE,
			WRITE_MODE,
			WRITE_DATA
		} m_currState = State::NONE;

		void UpdateState();

		BYTE m_stateRegister = 0;
		BYTE m_dataRegister = 0;
		BYTE m_statusRegister = 0;
		BYTE m_writeHandshakeRegister = 0;
		BYTE m_modeRegister = 0;
		bool m_sel = false;
	};
}

