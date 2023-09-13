#pragma once

#include <Video/Video.h>
#include <UI/Overlay.h>
#include "../ComputerMacintosh.h"
#include <CoreUI.h>

namespace ui
{
	class OverlayMac : public Overlay
	{
	public:
		OverlayMac();

		virtual bool Init() override;
		virtual void SetPC(emul::ComputerBase* pc) override;
		virtual bool Update() override;

	protected:
		emul::ComputerMacintosh* GetPC() { return (emul::ComputerMacintosh*)m_pc; }

		virtual void OnClick(CoreUI::WidgetRef widget) override;

		void UpdateFloppy(BYTE drive);

		void LoadFloppyDiskImage(BYTE drive);

		bool m_mouseCaptured = false;

		// UI Elements

		CoreUI::ToolbarItemPtr m_floppyButton[2];
		CoreUI::ToolbarItemPtr m_mouseButton;

		CoreUI::ImageRef m_floppyInactive = nullptr;
		CoreUI::ImageRef m_floppyActive = nullptr;

		CoreUI::ImageRef m_mouseCaptureOff = nullptr;
		CoreUI::ImageRef m_mouseCaptureOn = nullptr;
	};
}
