#pragma once

namespace emul::cpu68k
{
	class EventHandler
	{
	public:
		virtual void OnReset() {}
	};
}
