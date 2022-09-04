#include "stdafx.h"
#include "Computer.h"
#include <Config.h>
#include "CPU/CPUZ80.h"
#include "IO/InputEvents.h"

#include <assert.h>
#include <fstream>

using cfg::CONFIG;

namespace emul
{
	Computer::Computer(Memory& memory) :
		Logger("Computer")
	{
	}

	Computer::~Computer()
	{
		delete m_inputs;
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

	void Computer::Init(const char* cpuid, WORD baseram)
	{
		if (cpuid == CPUID_8080) m_cpu = new CPU8080(m_memory);
		else if (cpuid == CPUID_Z80) m_cpu = new CPUZ80(m_memory);
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

	void Computer::InitInputs(size_t clockSpeedHz, size_t pollingHz)
	{
		m_inputs = new events::InputEvents(clockSpeedHz, pollingHz);
		m_inputs->Init();
		m_inputs->EnableLog(CONFIG().GetLogLevel("inputs"));
	}

	void Computer::InitVideo(video::Video* video)
	{
		assert(video);
		m_video = video;
		m_video->Init(&m_memory, nullptr);
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
