#include "stdafx.h"

#include "DeviceGameBlaster.h"

namespace cms
{
	DeviceGameBlaster::DeviceGameBlaster(WORD baseAddress) :
		Logger("saa1099"),
		m_baseAddress(baseAddress),
		m_sound0(baseAddress + 0, "saa1099.0"),
		m_sound1(baseAddress + 2, "saa1099.1")
	{
		Reset();
	}

	DeviceGameBlaster::~DeviceGameBlaster()
	{
	}

	void DeviceGameBlaster::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		m_sound0.EnableLog(minSev);
		m_sound1.EnableLog(minSev);
	}

	void DeviceGameBlaster::Reset()
	{
	}

	void DeviceGameBlaster::Init()
	{
		Connect(m_baseAddress + 4, static_cast<PortConnector::INFunction>(&DeviceGameBlaster::Read4));

		Connect(m_baseAddress + 7, static_cast<PortConnector::OUTFunction>(&DeviceGameBlaster::WriteDetectByte));
		Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&DeviceGameBlaster::WriteDetectByte));

		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&DeviceGameBlaster::ReadDetectByte));
		Connect(m_baseAddress + 0xB, static_cast<PortConnector::INFunction>(&DeviceGameBlaster::ReadDetectByte));

		m_sound0.Init();
		m_sound1.Init();
	}

	BYTE DeviceGameBlaster::ReadDetectByte() 
	{ 
		LogPrintf(LOG_DEBUG, "ReadDetectByte");
		return m_detectByte; 
	}
	void DeviceGameBlaster::WriteDetectByte(BYTE value) 
	{
		LogPrintf(LOG_DEBUG, "WriteDetectByte, value=%02x", value);
		m_detectByte = value; 
	}

	// base+4 always return 0x7f
	BYTE DeviceGameBlaster::Read4() 
	{
		LogPrintf(LOG_DEBUG, "Read4");
		return 0x7F; 
	}

	void DeviceGameBlaster::Tick()
	{
		m_sound0.Tick();
		m_sound1.Tick();
	}

	saa1099::OutputData DeviceGameBlaster::GetOutput() const
	{
		saa1099::OutputData out;
		out.Mix(m_sound0.GetOutput());
		out.Mix(m_sound1.GetOutput());
		return out;
	}

	void DeviceGameBlaster::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;
		m_sound0.Serialize(to["saa1099.0"]);
		m_sound1.Serialize(to["saa1099.1"]);
	}

	void DeviceGameBlaster::Deserialize(const json& from)
	{
		WORD baseAddress = from["baseAddress"];
		if (baseAddress != m_baseAddress)
		{
			throw emul::SerializableException("DeviceGameBlaster: Incompatible baseAddress");
		}

		m_sound0.Deserialize(from["saa1099.0"]);
		m_sound1.Deserialize(from["saa1099.1"]);

	}
}
