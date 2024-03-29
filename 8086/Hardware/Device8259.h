#pragma once

#include <Serializable.h>
#include <CPU/PortConnector.h>

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace pic
{
	class Device8259 : public PortConnector, public emul::Serializable
	{
	public:
		Device8259(const char* id, WORD baseAddress, bool isPrimary = true);
		Device8259(WORD baseAddress, bool isPrimary = true);

		Device8259() = delete;
		Device8259(const Device8259&) = delete;
		Device8259& operator=(const Device8259&) = delete;
		Device8259(Device8259&&) = delete;
		Device8259& operator=(Device8259&&) = delete;

		void InterruptRequest(BYTE interrupt, bool value = true);

		void AttachSecondaryDevice(BYTE irq, Device8259* secondary);

		void Init();
		void Reset();

		void Tick();

		BYTE GetMask() { return m_interruptMaskRegister; }

		// TODO: simplification, doesn't handle multiple interrupts & priorities correctly
		bool InterruptPending() const;
		void InterruptAcknowledge();

		BYTE GetPendingInterrupt() const;

		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		const WORD m_baseAddress;

		enum class STATE
		{
			UNINITIALIZED,

			ICW1,
			ICW2,
			ICW3,
			ICW4,
			
			READY,
		} m_state = STATE::UNINITIALIZED;

		struct INIT
		{
			bool icw4Needed = false;
			bool single = false; // 1 = single, 0 = cascade
			bool levelTriggered = false; // 1 = level trigger, 0 = edge trigger
			BYTE interruptBase = 0; // Upper 5 bits of interrupt sent to cpu
			bool cpu8086 = false; // 1 = 8086/8088, 0 = 8080/8085
			bool autoEOI = false; // 1 = Auto EOI (End of Interrupt), 0 = Normal EOI
			bool buffered = false; // 1 = Buffered Mode
			bool sfnm = false; // 1 = Special Fully Nested Mode

			void Reset();
		} m_init;

		BYTE Read0();
		void Write0(BYTE value);

		BYTE Read1();
		void Write1(BYTE value);

		void OCW2(BYTE value);
		void OCW3(BYTE value);
		
		void NonSpecificEOI();
		void SpecificEOI(BYTE level);

		BYTE m_lastInterruptRequestRegister = 0xFF; // For edge detection
		BYTE m_interruptRequestRegister = 0;

		BYTE m_inServiceRegister = 0;
		BYTE m_interruptMaskRegister = 0;

		BYTE* m_reg0 = &m_inServiceRegister; // points to IR or IS depending on last OCW3 command

		// Cascade mode
		void SetPrimary(Device8259* primary) { m_cascade.primaryDevice = primary; }
		struct Cascade
		{
			bool primary = true;

			// Primary config
			std::array<bool, 8> secondaryConnected = { false, false, false, false, false, false, false };
			std::array<Device8259*, 8> secondaryDevices = { 0, 0, 0, 0, 0, 0, 0 };

			// Secondary config
			Device8259* primaryDevice = nullptr;
			BYTE primaryIRQ = 0;

			void Reset();
		} m_cascade;
	};
}
