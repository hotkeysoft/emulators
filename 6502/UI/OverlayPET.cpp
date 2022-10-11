#include "stdafx.h"

#include "OverlayPET.h"
#include "ComputerPET2001.h"

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

#include <commdlg.h>

using namespace CoreUI;
namespace fs = std::filesystem;

namespace ui
{
	void OverlayPET::SetPC(emul::ComputerBase* pc)
	{
		Overlay::SetPC(pc);

		ToolbarItemPtr item = GetToolbar()->AddToolbarItem("loadPRG", RES().FindImage("overlay16", 2), "PRG");
		item->SetTooltip("Load PRG software");
	}

	void OverlayPET::OnClick(CoreUI::WidgetRef widget)
	{
		const std::string& id = widget->GetId();

		if (id == "loadPRG")
		{
			LoadPRG();
		}
		else
		{
			// Let parent handle it
			Overlay::OnClick(widget);
		}
	}

	void OverlayPET::LoadPRG()
	{
		if (!m_pc)
		{
			return;
		}

		emul::ComputerPET2001* pet = dynamic_cast<emul::ComputerPET2001*>(m_pc);
		if (!pet)
		{
			LogPrintf(LOG_ERROR, "Invalid architecture");
			return;
		}

		fs::path diskImage;

		if (SelectFile(diskImage, GetHWND()))
		{
			pet->LoadPRG(diskImage.string().c_str());
		}
	}
}