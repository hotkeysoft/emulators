#pragma once

namespace hscommon
{
	class EdgeDetectLatch
	{
	public:
		// POSITIVE: Latches on 0->1 transition
		// NEGATIVE: Latches on 1->0 transition
		enum class Trigger { POSITIVE, NEGATIVE };
		EdgeDetectLatch(Trigger t = Trigger::POSITIVE) : m_invert(t == Trigger::NEGATIVE) {}

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

	private:
		bool m_invert = false;
		bool m_value = true;
		bool m_latched = false;
	};
}
