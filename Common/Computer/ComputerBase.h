#pragma once

#include <CPU/CPU.h>
#include <CPU/Memory.h>
#include <Serializable.h>
#include "Video/Video.h"
#include <set>

using emul::WORD;

namespace events { class InputEvents; }
namespace tape { class DeviceTape; }

namespace emul
{
	class ComputerBase : public Serializable, public PortConnector
	{
	public:
		virtual ~ComputerBase();

		virtual std::string_view GetName() const = 0;
		virtual std::string_view GetID() const = 0;

		virtual void Init(WORD baseRAM) = 0;

		virtual bool Step() { return m_cpu->Step(); }

		virtual void Reset() { m_cpu->Reset(); }

		CPU* GetCPU() const { return m_cpu; }
		Memory& GetMemory() { return m_memory; }
		events::InputEvents& GetInputs() { return *m_inputs; }
		video::Video& GetVideo() { return *m_video; }

		virtual tape::DeviceTape* GetTape() { return nullptr; }

		virtual void Reboot();
		void SetTurbo(bool turbo) { m_turbo = turbo; }

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		ComputerBase(Memory& memory, WORD blockGranularity = 1024);

		virtual void Init(const char* cpuid, WORD baseRAM);
		virtual void InitCPU(const char* cpuID) = 0;
		virtual void InitInputs(size_t clockSpeedHz, size_t pollInterval = 0);

		Memory m_memory;

		WORD m_baseRAMSize = 0;

		emul::CPU* m_cpu = nullptr;
		events::InputEvents* m_inputs = nullptr;
		video::Video* m_video = nullptr;

		bool m_turbo = false;
	};
}
