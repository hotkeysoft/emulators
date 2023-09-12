#pragma once

#include <Serializable.h>
#include <CPU/CPUCommon.h>
#include <CPU/IOConnector.h>
#include <EdgeDetectLatch.h>

using emul::IOConnector;
using emul::BYTE;
using emul::WORD;

namespace scc
{
	class Device8530;

	static constexpr int REG_COUNT = 16;

	class SCCChannel : public Logger, public emul::Serializable
	{
	public:
		SCCChannel(std::string id);

		void Init(Device8530* parent);

		void Reset(bool hard);

		BYTE ReadData();
		void WriteData(BYTE value);

		BYTE ReadControl();
		void WriteControl(BYTE value);

		bool GetDCD() const { return m_DCD; }
		void SetDCD(bool set);

		bool IsExternalStatusChange() const { return m_extStatusChange; }
		bool IsInterrupt() const;

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		bool m_extStatusChange = false;
		bool m_DCD = false;
		bool m_DCDInt = false;

		void WR0(BYTE value);
		void WR1(BYTE value);
		void WR3(BYTE value);
		void WR4(BYTE value);
		void WR5(BYTE value);
		void WR6(BYTE value);
		void WR7(BYTE value);
		void WR10(BYTE value);
		void WR11(BYTE value);
		void WR12(BYTE value);
		void WR13(BYTE value);
		void WR14(BYTE value);
		void WR15(BYTE value);

		emul::BYTE RR0();
		emul::BYTE RR1();
		emul::BYTE RR3();
		emul::BYTE RR8();
		emul::BYTE RR10();
		emul::BYTE RR12();
		emul::BYTE RR13();
		emul::BYTE RR15();

		struct Registers
		{
			BYTE WR0 = 0;
			BYTE WR1 = 0;
			BYTE WR3 = 0;
			BYTE WR4 = 0;
			BYTE WR15 = 0;

			BYTE RR0 = 0;
		} m_regs;

		Device8530* m_parent = nullptr;
	};

	// 8530 SCC
	class Device8530 : public Logger, public emul::Serializable
	{
	public:
		Device8530(std::string id = "SCC");
		virtual ~Device8530() {}

		Device8530(const Device8530&) = delete;
		Device8530& operator=(const Device8530&) = delete;
		Device8530(Device8530&&) = delete;
		Device8530& operator=(Device8530&&) = delete;

		virtual void Init();
		virtual void Reset(bool hard = true);
		virtual void EnableLog(SEVERITY minSev = LOG_INFO) override;

		const SCCChannel& GetChannelA() const { return m_channelA; }
		const SCCChannel& GetChannelB() const { return m_channelB; }

		SCCChannel& GetChannelA() { return m_channelA; }
		SCCChannel& GetChannelB() { return m_channelB; }

		virtual void OnReadChannel (SCCChannel* src) {};
		virtual void OnWriteChannel(SCCChannel* src) {};

		bool GetIRQ() const { return m_channelA.IsInterrupt() || m_channelB.IsInterrupt(); }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		SCCChannel m_channelA;
		SCCChannel m_channelB;

		void WR2(BYTE value);
		void WR9(BYTE value);

		emul::BYTE RR2(SCCChannel* ch);

		void SetCurrRegister(int reg) { assert(reg >= 0 && reg < REG_COUNT); m_currRegister = reg; }
		int GetCurrRegister() const { return m_currRegister; }

	private:
		int m_currRegister = 0;

		struct Registers
		{
			BYTE WR2 = 0;
			BYTE WR9 = 0;
		} m_regs;

		friend class SCCChannel;
	};
}
