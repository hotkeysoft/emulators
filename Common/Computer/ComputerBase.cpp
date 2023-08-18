#include "stdafx.h"
#include <Computer/ComputerBase.h>
#include <Config.h>
#include "IO/InputEvents.h"

#include <assert.h>
#include <fstream>

using cfg::CONFIG;

namespace emul
{
	ComputerBase::ComputerBase(Memory& memory, WORD blockGranularity) :
		Logger("Computer"),
		m_memory(blockGranularity)
	{
	}

	ComputerBase::~ComputerBase()
	{
		delete m_inputs;
		delete m_cpu;
		delete m_video;
	}

	void ComputerBase::Reboot()
	{
		LogPrintf(LOG_WARNING, "Reset");
		m_cpu->Reset();
		Reset();
	}

	void ComputerBase::Init(const char* cpuid, WORD baseram)
	{
		m_baseRAMSize = baseram;

		InitCPU(cpuid);
		if (!m_cpu)
		{
			throw std::exception("CPU not set");
		}

		m_cpu->EnableLog(CONFIG().GetLogLevel("cpu"));
		m_cpu->Init();

		PortConnector::Clear();

		m_memory.Init(m_cpu->GetAddressBits());
		m_memory.EnableLog(CONFIG().GetLogLevel("memory"));
	}

	void ComputerBase::InitInputs(size_t clockSpeedHz, size_t pollInterval)
	{
		assert(clockSpeedHz > 0);
		if (pollInterval < 1 || pollInterval >= clockSpeedHz)
		{
			pollInterval = clockSpeedHz / 60; // Default 60Hz polling
		}
		m_inputs = new events::InputEvents(clockSpeedHz, pollInterval);
		m_inputs->Init();
		m_inputs->EnableLog(CONFIG().GetLogLevel("inputs"));
	}

	void ComputerBase::Serialize(json& to)
	{
		json computer;

		computer["id"] = GetID();
		computer["baseram"] = m_baseRAMSize;
		to["computer"] = computer;


		m_cpu->Serialize(to["cpu"]);
		m_memory.Serialize(to["memory"]);
		m_video->Serialize(to["video"]);
	}

	void ComputerBase::Deserialize(const json& from)
	{
		const json& computer = from["computer"];
		if ((std::string)computer["id"] != GetID())
		{
			throw SerializableException("Computer: Architecture is not compatible", SerializationError::COMPAT);
		}
		if (computer["baseram"] != m_baseRAMSize)
		{
			throw SerializableException("Computer: Base RAM is not compatible", SerializationError::COMPAT);
		}

		m_cpu->Deserialize(from["cpu"]);
		m_memory.Deserialize(from["memory"]);
		m_video->Deserialize(from["video"]);
	}
}
