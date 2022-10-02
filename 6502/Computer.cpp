#include "stdafx.h"
#include "Computer.h"
#include <Config.h>
#include "CPU/CPU6502.h"
#include "IO/InputEvents.h"

#include <assert.h>
#include <fstream>

using cfg::CONFIG;

namespace emul
{
	Computer::Computer(Memory& memory) :
		Logger("Computer"),
		m_memory(256)
	{
	}

	Computer::~Computer()
	{
		delete m_inputs;
		delete m_cpu;
	}

	void Computer::Reboot()
	{
		LogPrintf(LOG_WARNING, "Reset");
		m_cpu->Reset();
		m_memory.Clear();
	}

	void Computer::Init(const char* cpuid, WORD baseram)
	{
		if (cpuid == CPUID_6502) m_cpu = new CPU6502(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}

		m_cpu->EnableLog(CONFIG().GetLogLevel("cpu"));
		m_cpu->Init();

		m_memory.Init(m_cpu->GetAddressBits());

		PortConnector::Clear();
	}

	void Computer::InitInputs(size_t clockSpeedHz, size_t pollInterval)
	{
		m_inputs = new events::InputEvents(clockSpeedHz, pollInterval);
		m_inputs->Init();
		m_inputs->EnableLog(CONFIG().GetLogLevel("inputs"));
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
