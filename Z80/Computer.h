#pragma once

#include "CPU/CPU8080.h"
#include "CPU/Memory.h"

#include "Video/Video.h"
#include "Serializable.h"

#include <set>

using emul::WORD;

namespace emul
{
	class Computer : public Serializable, public PortConnector
	{
	public:
		virtual ~Computer();

		virtual std::string_view GetName() const = 0;
		virtual std::string_view GetID() const = 0;

		virtual void Init(WORD baseRAM) = 0;

		virtual bool Step() { return m_cpu->Step(); }

		virtual void Reset() { m_cpu->Reset(); }

		bool LoadBinary(const char* file, ADDRESS baseAddress) { return m_memory.LoadBinary(file, baseAddress); }
		
		CPU8080& GetCPU() const { return *m_cpu; }
		Memory& GetMemory() { return m_memory; }
		video::Video& GetVideo() { return *m_video; }

		virtual void Reboot();
		void SetTurbo(bool turbo) { m_turbo = turbo; }

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		Computer(Memory& memory);

		virtual void Init(cpuInfo::CPUType cpu, WORD baseRAM);

		Memory m_memory;

		emul::CPU8080* m_cpu = nullptr;
		video::Video* m_video = nullptr;

		bool m_turbo = false;
	};
}
