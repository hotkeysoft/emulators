#pragma once

#ifndef HSCOMMON_NO_SERIALIZE
#include <Serializable.h>
#endif

namespace hscommon
{
	class EdgeDetectLatch
#ifndef HSCOMMON_NO_SERIALIZE
		: public emul::Serializable
#endif
	{
	public:
		// POSITIVE: Latches on 0->1 transition
		// NEGATIVE: Latches on 1->0 transition
		enum class Trigger { POSITIVE, NEGATIVE };
		EdgeDetectLatch(Trigger t = Trigger::POSITIVE) { SetTrigger(t); }

		void Set(bool set)
		{
			if (m_invert) { set = !set; };

			if (!m_value && set)
			{
				m_latched = true;
			}
			m_value = set;
		}

		void ResetLatch() { m_latched = false; }
		bool IsLatched() const { return m_latched; }

		void SetTrigger(Trigger t) { ResetLatch(); m_invert = (t == Trigger::NEGATIVE); }

#ifndef HSCOMMON_NO_SERIALIZE
		// emul::Serializable
		virtual void Serialize(json& to) override
		{
			to["invert"] = m_invert;
			to["value"] = m_value;
			to["latched"] = m_latched;
		}
		virtual void Deserialize(const json& from) override
		{
			m_invert = from["invert"];
			m_value = from["value"];
			m_latched = from["latched"];
		}
#endif
	private:
		bool m_invert = false;
		bool m_value = true;
		bool m_latched = false;
	};
}
