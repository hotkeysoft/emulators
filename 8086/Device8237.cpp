#include "Device8237.h"
#include "PortAggregator.h"

namespace dma
{
	DMAChannel::DMAChannel(Device8237* parent, WORD id, const char* label) :
		Logger(label), 
		m_parent(parent),
		m_id(id),
		m_address(0x1234),
		m_count(0x2345)
	{
	}

	void DMAChannel::Init()
	{
		Connect(m_parent->GetBaseAdress() + (m_id * 2), static_cast<PortConnector::INFunction>(&DMAChannel::ADDR_IN));
		Connect(m_parent->GetBaseAdress() + (m_id * 2), static_cast<PortConnector::OUTFunction>(&DMAChannel::ADDR_OUT));

		Connect(m_parent->GetBaseAdress() + (m_id * 2) + 1, static_cast<PortConnector::INFunction>(&DMAChannel::COUNT_IN));
		Connect(m_parent->GetBaseAdress() + (m_id * 2) + 1, static_cast<PortConnector::OUTFunction>(&DMAChannel::COUNT_OUT));
	}
	void DMAChannel::Reset()
	{
	}

	void DMAChannel::Tick()
	{
		// Fake memory refresh
		//++m_address;
	}

	BYTE DMAChannel::ADDR_IN()
	{
		LogPrintf(LOG_DEBUG, "Read ADDR, value = %02X", m_address);
		return m_address;
	}
	void DMAChannel::ADDR_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write ADDR, value=%02X", value);
		m_address = value;
	}

	BYTE DMAChannel::COUNT_IN()
	{
		LogPrintf(LOG_DEBUG, "Read COUNT");
		return m_count;
	}

	void DMAChannel::COUNT_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write COUNT, value=%02X", value);
		m_count = value;
	}

	Device8237::Device8237(WORD baseAddress) :
		Logger("dma"),
		m_channel0(this, 0, "dma.0"),
		m_channel1(this, 1, "dma.1"),
		m_channel2(this, 2, "dma.2"),
		m_channel3(this, 3, "dma.3"),
		m_baseAddress(baseAddress)
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

	}

	void Device8237::Init()
	{
		m_channel0.Init();
		m_channel1.Init();
		m_channel2.Init();
		m_channel3.Init();

		// Dummy control/etc channels
		for (int i = 0; i < 8; ++i)
		{
			Connect(GetBaseAdress() + 8 + i, static_cast<PortConnector::INFunction>(&Device8237::TEMP_IN));
			Connect(GetBaseAdress() + 8 + i, static_cast<PortConnector::OUTFunction>(&Device8237::TEMP_OUT));
		}
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

	BYTE Device8237::TEMP_IN()
	{
		LogPrintf(LOG_INFO, "TEMP IN");
		return 0;
	}
	void Device8237::TEMP_OUT(BYTE value)
	{
		LogPrintf(LOG_INFO, "TEMP OUT, value=%02X", value);
	}
}
