#pragma once

namespace vid_events
{
	class EventHandler
	{
	public:
		virtual void OnRenderFrame() {}
		virtual void OnNewFrame() {}
		virtual void OnEndOfRow() {}
		virtual void OnChangeMode() {}
	};

	static EventHandler s_defaultHandler;

	class EventSource
	{
	public:
		void SetVidEventHandler(EventHandler* handler) { m_vidEventHandler = handler; }

	protected:
		void FireRenderFrame() { m_vidEventHandler->OnRenderFrame(); }
		void FireNewFrame() { m_vidEventHandler->OnNewFrame(); }
		void FireEndOfRow() { m_vidEventHandler->OnEndOfRow(); }
		void FireChangeMode() { m_vidEventHandler->OnChangeMode(); }

	private:
		EventHandler* m_vidEventHandler = &s_defaultHandler;
	};
}
