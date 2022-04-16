#pragma once

#include "../Serializable.h"
#include "../CPU/PortConnector.h"
#include "DeviceSAA1099.h"

using emul::PortConnector;

namespace cms
{
	class DeviceGameBlaster : public PortConnector, public emul::Serializable
	{
	public:
		DeviceGameBlaster(WORD baseAddress);
		~DeviceGameBlaster();

		DeviceGameBlaster() = delete;
		DeviceGameBlaster(const DeviceGameBlaster&) = delete;
		DeviceGameBlaster& operator=(const DeviceGameBlaster&) = delete;
		DeviceGameBlaster(DeviceGameBlaster&&) = delete;
		DeviceGameBlaster& operator=(DeviceGameBlaster&&) = delete;

		virtual void EnableLog(SEVERITY minSev = LOG_INFO);

		void Init();
		void Reset();

		void Tick();

		saa1099::OutputData GetOutput() const;

		saa1099::DeviceSAA1099& GetSound0() { return m_sound0; }
		saa1099::DeviceSAA1099& GetSound1() { return m_sound1; }

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		const WORD m_baseAddress;

		// Used for card detection: value written at base+7|base+8 is latched
		// and can be read back at base+a|base+b
		BYTE ReadDetectByte();
		void WriteDetectByte(BYTE value);
		BYTE m_detectByte = 0;
		// Also base+4 always return 0x7f
		BYTE Read4();

		saa1099::DeviceSAA1099 m_sound0;
		saa1099::DeviceSAA1099 m_sound1;
	};
}
