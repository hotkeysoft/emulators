#include "stdafx.h"

#include "OverlayPET.h"
#include "PRGLoader.h"

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
#include <FileUtil.h>

#include <commdlg.h>

using namespace CoreUI;
using namespace hscommon::fileUtil;

namespace fs = std::filesystem;
using emul::PRGLoader;

namespace ui
{
	void OverlayPET::SetPC(emul::ComputerBase* pc)
	{
		Overlay::SetPC(pc);

		if (PRGLoader* prg = GetPRGLoader(); prg != nullptr)
		{
			ToolbarItemPtr item = GetToolbar()->AddToolbarItem("loadPRG", RES().FindImage("overlay16", 2), "PRG");
			item->SetTooltip("Load PRG software (BASIC programs or cartridge ROMs)");

			if (prg->CanUnloadPRG())
			{
				item = GetToolbar()->AddToolbarItem("unloadPRG", RES().FindImage("overlay16", 7));
				item->SetTooltip("Eject cartridge and\nRESET the computer");
			}

			GetToolbar()->AddSeparator();
		}
	}

	void OverlayPET::OnClick(CoreUI::WidgetRef widget)
	{
		const std::string& id = widget->GetId();

		if (id == "loadPRG")
		{
			LoadPRG();
		}
		else if (id == "unloadPRG")
		{
			UnloadPRG();
		}
		else
		{
			// Let parent handle it
			Overlay::OnClick(widget);
		}
	}

	void OverlayPET::LoadPRG()
	{
		emul::PRGLoader* prgLoader = GetPRGLoader();
		if (!prgLoader)
		{
			LogPrintf(LOG_ERROR, "Invalid architecture");
			return;
		}

		PathList paths;

		if (SelectFileMulti(paths, { {"Program Files (*.prg)", "*.prg"} }))
		{
			prgLoader->LoadPRG(paths);
		}
	}

	void OverlayPET::UnloadPRG()
	{
		emul::PRGLoader* prgLoader = GetPRGLoader();
		if (!prgLoader)
		{
			LogPrintf(LOG_ERROR, "Invalid architecture");
			return;
		}

		prgLoader->UnloadPRG();
	}
}
