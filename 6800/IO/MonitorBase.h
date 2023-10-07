#pragma once

#include <IO/Console.h>
#include <CPU/CPU.h>
#include <CPU/Memory.h>
#include <CPU/CPUInfo.h>

namespace emul
{
	class CPU;

	enum class MonitorState { RUN, WAIT, EXIT, SWITCH_MODE };
	class MonitorBase
	{
	public:
		MonitorBase(Console& console) : m_console(console) {}
		virtual ~MonitorBase() {}

		MonitorBase() = delete;
		MonitorBase(const MonitorBase&) = delete;
		MonitorBase& operator=(const MonitorBase&) = delete;
		MonitorBase(MonitorBase&&) = delete;
		MonitorBase& operator=(MonitorBase&&) = delete;

		virtual void Init(CPU* cpu, Memory& memory) = 0;

		void SetCustomMemoryView(ADDRESS address) { m_customMemView = address; }
		void SetBreakpoint(ADDRESS address) { m_breakpoint = address; m_breakpointEnabled = true; }
		void ClearBreakpoint() { m_breakpointEnabled = false; }
		bool IsBreakpoint() const { return m_breakpointEnabled && m_cpu->GetCurrentAddress() == m_breakpoint; }

		virtual void Show() = 0;
		virtual MonitorState Run() = 0;
		virtual void Update() = 0;
		void SetStepMode() { m_runMode = RUNMode::STEP; }

	protected:
		ADDRESS m_breakpoint = 0;
		bool m_breakpointEnabled = false;

		enum class RUNMode { STEP, RUN };
		RUNMode m_runMode = RUNMode::STEP;

		ADDRESS m_customMemView = 0;			
		Console& m_console;
		CPU* m_cpu = nullptr;
		Memory* m_memory = nullptr;

		static bool Replace(std::string& str, const std::string& from, const std::string& to);

		void WriteValueNibble(BYTE value, const cpuInfo::Coord& coord, WORD attr = 15);
		void WriteValueHex(BYTE value, const cpuInfo::Coord& coord, WORD attr = 15);
		void WriteValueHex(WORD value, const cpuInfo::Coord& coord, WORD attr = 15);
	};
}
