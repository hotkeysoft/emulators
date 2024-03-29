#pragma once

#include <Video/Video.h>
#include <UI/Overlay.h>
#include "../ComputerCPC.h"
#include <CoreUI.h>

namespace ui
{
	using NewComputerCallback = void (*)(std::filesystem::path, json& data);

	class OverlayCPC : public Overlay
	{
	public:
		OverlayCPC();

		virtual bool Init() override;
		virtual void SetPC(emul::ComputerBase* pc) override;
		virtual bool Update() override;

	protected:
		emul::ComputerCPC* GetPC() { return (emul::ComputerCPC*)m_pc; }

		virtual void OnClick(CoreUI::WidgetRef widget) override;

		void UpdateFloppy(BYTE drive);

		void LoadFloppyDiskImage(BYTE drive, bool eject = false);

		// UI Elements
		CoreUI::ToolbarItemPtr m_floppyButton[2];
		CoreUI::ToolbarItemPtr m_ejectButton[2];

		CoreUI::ImageRef m_floppyInactive = nullptr;
		CoreUI::ImageRef m_floppyActive = nullptr;
	};
}
