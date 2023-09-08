#pragma once
#include <CPU/Memory.h>
#include <Serializable.h>

namespace emul
{
	class CPU;
	typedef void(*CPUCallbackFunc)(CPU* cpu, ADDRESS addr);

	struct WatchItem
	{
		ADDRESS addr;
		CPUCallbackFunc onCall;
		CPUCallbackFunc onRet;
	};

	enum class CPUState { STOP, RUN, STEP, HALT };
	class CPU : virtual public Logger, public Serializable
	{
	public:
		CPU(size_t addressBits, Memory& memory);
		virtual ~CPU();

		virtual const std::string GetID() const = 0;
		virtual size_t GetAddressBits() const = 0;
		virtual ADDRESS GetCurrentAddress() const = 0;
		virtual ADDRESS GetLastAddress() const { return m_lastAddress; }
		virtual void Exec(BYTE opcode) = 0;

		virtual void Init() = 0;
		virtual void Reset();
		void Run();
		virtual bool Step();
		virtual void Halt() { m_state = CPUState::HALT; }

		uint32_t GetInstructionTicks() const { return m_opTicks; }

		CPUState GetState() const { return m_state; }

		// emul::Serializable
		virtual void Serialize(json& to) {};
		virtual void Deserialize(const json& from) {};

	protected:
		virtual BYTE FetchByte() = 0;
		virtual WORD FetchWord();

		CPUState m_state = CPUState::STOP;
		Memory& m_memory;

		uint32_t m_opTicks = 0;
		ADDRESS m_lastAddress = 0;
		inline void TICK(uint32_t count) { m_opTicks += count; }
	};
}
