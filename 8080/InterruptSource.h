#pragma once

namespace emul
{
	class InterruptSource : virtual public Logger
	{
	public:
		InterruptSource() {}
		virtual ~InterruptSource() {}

		virtual bool IsInterrupting() = 0;
	};
}