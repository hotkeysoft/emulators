#pragma once

#include "../Serializable.h"
#include "../CPU/PortConnector.h"

using emul::PortConnector;

namespace dss
{
	const size_t FIFO_FREQUENCY = 70000;

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

		void WriteData(BYTE value);
		void WriteControl(BYTE value);
		BYTE ReadStatus();
	};
}
