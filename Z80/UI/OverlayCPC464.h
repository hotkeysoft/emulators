#pragma once

#include <Video/Video.h>
#include <UI/Overlay.h>
#include "../ComputerCPC464.h"
#include <CoreUI.h>

namespace ui
{
	using NewComputerCallback = void (*)(std::filesystem::path, json& data);

	class OverlayCPC464 : public Overlay
	{
	public:
		OverlayCPC464();

		virtual bool Init() override;
		virtual void SetPC(emul::ComputerBase* pc) override;
		virtual bool Update() override;

	protected:
		emul::ComputerCPC464* GetPC() { return (emul::ComputerCPC464*)m_pc; }

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
