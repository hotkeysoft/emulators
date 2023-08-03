#include "stdafx.h"

#include <UI/MainWindow.h>
#include "OverlayCPC.h"
#include <Config.h>
#include "../Storage/DeviceFloppy.h"

#pragma warning(disable:4251)

#include <CPU/CPUCommon.h>
#include <Core/Window.h>
#include <Core/WindowManager.h>
#include <Core/ResourceManager.h>
#include <Core/Widget.h>
#include <Core/Tooltip.h>
#include <Widgets/Toolbar.h>
#include <Widgets/ToolbarItem.h>
#include <Widgets/Button.h>
#include <Widgets/Label.h>
#include <Widgets/TextBox.h>
#include <FileUtil.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_syswm.h>

namespace fs = std::filesystem;
using cfg::CONFIG;
using namespace CoreUI;
using namespace hscommon::fileUtil;

namespace ui
{
	OverlayCPC::OverlayCPC()
	{
	}

	bool OverlayCPC::Init()
	{
		if (!Overlay::Init())
			return false;

		m_floppyInactive = RES().FindImage("overlay16", 0);
		m_floppyActive = RES().FindImage("overlay16", 4);
		return true;
	}

	void OverlayCPC::SetPC(emul::ComputerBase* pc)
	{
		Overlay::SetPC(pc);

		emul::ComputerCPC* pcCPC = dynamic_cast<emul::ComputerCPC*>(pc);
		assert(pcCPC != nullptr);

		// Toolbar section: Floppy drives
		if (GetPC()->GetFloppy())
		{
			m_floppyButton[0] = GetToolbar()->AddToolbarItem("floppy0", m_floppyInactive, "A:");
			m_ejectButton[0] = GetToolbar()->AddToolbarItem("eject0", RES().FindImage("overlay16", 7));
			m_ejectButton[0]->SetTooltip("Eject A:");
			UpdateFloppy(0);

			m_floppyButton[1] = GetToolbar()->AddToolbarItem("floppy1", m_floppyInactive, "B:");
			m_ejectButton[1] = GetToolbar()->AddToolbarItem("eject1", RES().FindImage("overlay16", 7));
			m_ejectButton[1]->SetTooltip("Eject B:");
			UpdateFloppy(1);

			GetToolbar()->AddSeparator();
		}
		else
		{
			m_floppyButton[0] = nullptr;
			m_floppyButton[1] = nullptr;
			m_ejectButton[0] = nullptr;
			m_ejectButton[1] = nullptr;
		}
	}

	void OverlayCPC::OnClick(CoreUI::WidgetRef widget)
	{
		const std::string& id = widget->GetId();

		if (id == "floppy0")
		{
			LoadFloppyDiskImage(0);
		}
		else if (id == "floppy1")
		{
			LoadFloppyDiskImage(1);
		}
		else if (id == "eject0")
		{
			LoadFloppyDiskImage(0, true);
		}
		else if (id == "eject1")
		{
			LoadFloppyDiskImage(1, true);
		}
		else
		{
			// Let parent handle it
			Overlay::OnClick(widget);
		}
	}

	void OverlayCPC::UpdateFloppy(BYTE drive)
	{
		const auto& image = GetPC()->GetFloppy()->GetImageInfo(drive);

		std::ostringstream os;
		os << (char)('A'+drive) << ':';
		if (image.loaded)
		{
			m_floppyButton[drive]->SetTooltip(image.path.stem().string().c_str());

			os << " ["
				<< image.geometry.name
				<< "]";
		}
		else
		{
			m_floppyButton[drive]->SetTooltip("Load floppy image");
		}

		m_floppyButton[drive]->SetText(os.str().c_str());
	}

	bool OverlayCPC::Update()
	{
		if (!Overlay::Update())
		{
			return false;
		}

		if (GetPC()->GetFloppy())
		{
			for (int i = 0; i < 2; ++i)
			{
				m_floppyButton[i]->SetImage(GetPC()->GetFloppy()->IsActive(i) ? m_floppyActive : m_floppyInactive);
			}
		}

		return true;
	}

	void OverlayCPC::LoadFloppyDiskImage(BYTE drive, bool eject)
	{
		if (!GetPC() || !GetPC()->GetFloppy())
		{
			return;
		}

		fs::path diskImage;

		if (eject)
		{
			GetPC()->GetFloppy()->ClearDiskImage(drive);
		}
		else if (SelectFile(diskImage, { {"Floppy disk image (*.dsk)", "*.dsk"} }))
		{
			GetPC()->GetFloppy()->LoadDiskImage(drive, diskImage.string().c_str());
		}
		UpdateFloppy(drive);
	}
}
