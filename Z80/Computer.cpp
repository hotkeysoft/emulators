#include "stdafx.h"
#include "Computer.h"
#include "Config.h"
#include "CPU/CPUZ80.h"

#include <assert.h>
#include <fstream>

using cpuInfo::CPUType;
using cfg::CONFIG;

namespace emul
{
	Computer::Computer(Memory& memory) :
		Logger("Computer")
	{
	}

	Computer::~Computer()
	{
		delete m_video;
		delete m_cpu;
	}

	void Computer::Reboot()
	{
		LogPrintf(LOG_WARNING, "Reset");			
		m_cpu->Reset();
		m_memory.Clear();

		// TODO: Reset all components
		m_video->Reset();
	}

	void Computer::Init(CPUType type, WORD baseram)
	{
		switch (type)
		{
		case CPUType::i8080: m_cpu = new CPU8080(m_memory, m_interrupts); break;
		case CPUType::z80: m_cpu = new CPUZ80(m_memory, m_interrupts); break;
		default:
			LogPrintf(LOG_ERROR, "CPUType not supported");
			throw std::exception("CPUType not supported");
		}
	
		m_cpu->EnableLog(CONFIG().GetLogLevel("cpu"));
		m_cpu->Init();

		m_memory.Init(m_cpu->GetAddressBits());

		PortConnector::Clear();
	}

	void Computer::Serialize(json& to)
	{
		json computer;

		computer["id"] = GetID();
		to["computer"] = computer;

		m_cpu->Serialize(to["cpu"]);
		m_memory.Serialize(to["memory"]);
	}

	void Computer::Deserialize(const json& from)
	{
		const json& computer = from["computer"];
		if ((std::string)computer["id"] != GetID())
		{
			throw SerializableException("Computer: Architecture is not compatible", SerializationError::COMPAT);
		}

		m_cpu->Deserialize(from["cpu"]);
		m_memory.Deserialize(from["memory"]);
	}
}