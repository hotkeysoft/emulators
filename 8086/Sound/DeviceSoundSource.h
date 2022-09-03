#pragma once

#include <Serializable.h>
#include "../CPU/PortConnector.h"
#include <queue>

using emul::PortConnector;

namespace dss
{
	const size_t FIFO_FREQUENCY = 7000;

	class DeviceSoundSource : public PortConnector, public emul::Serializable
	{
	public:
		DeviceSoundSource(WORD baseAddress, size_t baseFreq);
		~DeviceSoundSource();

		DeviceSoundSource() = delete;
		DeviceSoundSource(const DeviceSoundSource&) = delete;
		DeviceSoundSource& operator=(const DeviceSoundSource&) = delete;
		DeviceSoundSource(DeviceSoundSource&&) = delete;
		DeviceSoundSource& operator=(DeviceSoundSource&&) = delete;

		void Init();
		void Reset();

		void Tick();

		WORD GetOutput() const;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		const size_t m_tickDivider;
		const WORD m_baseAddress;

		bool m_select = false;
		BYTE m_currData = 0;
		BYTE m_output = 0;

		void Push(BYTE value);
		BYTE Pop();

		std::queue<BYTE> m_fifo;
		bool m_fifoFull = false;

		void WriteData(BYTE value);
		BYTE ReadData();
		void WriteControl(BYTE value);
		BYTE ReadStatus();
	};
}
