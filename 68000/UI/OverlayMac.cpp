#include "stdafx.h"

#include <UI/MainWindow.h>
#include "OverlayMac.h"
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
	OverlayMac::OverlayMac()
	{
	}

	bool OverlayMac::Init()
	{
		if (!Overlay::Init())
			return false;

		m_floppyInactive = RES().FindImage("overlay16", 0);
		m_floppyActive = RES().FindImage("overlay16", 4);
		m_mouseCaptureOff = RES().FindImage("overlay16", 16);
		m_mouseCaptureOn = RES().FindImage("overlay16", 17);
		return true;
	}

	void OverlayMac::SetPC(emul::ComputerBase* pc)
	{
		Overlay::SetPC(pc);

		emul::ComputerMacintosh* pcMac = dynamic_cast<emul::ComputerMacintosh*>(pc);
		assert(pcMac != nullptr);

		// Toolbar section: Floppy drives
		if (GetPC()->GetFloppy(0).IsConnected())
		{
			m_floppyButton[0] = GetToolbar()->AddToolbarItem("floppy0", m_floppyInactive, "Internal");
			UpdateFloppy(0);
		}
		else
		{
			m_floppyButton[0] = nullptr;
		}

		if (GetPC()->GetFloppy(1).IsConnected())
		{
			m_floppyButton[1] = GetToolbar()->AddToolbarItem("floppy1", m_floppyInactive, "External");
			UpdateFloppy(1);
		}
		else
		{
			m_floppyButton[1] = nullptr;
		}

		GetToolbar()->AddSeparator();

		// Mouse
		m_mouseButton = GetToolbar()->AddToolbarItem("mouse", m_mouseCaptureOff);
		m_mouseButton->SetTooltip("Toggle Mouse Capture\nKeyboard: [Scroll Lock] key");

		GetToolbar()->AddSeparator();
	}

	void OverlayMac::OnClick(CoreUI::WidgetRef widget)
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
		else if (id == "mouse")
		{
			if (m_pc)
			{
				m_pc->GetInputs().CaptureMouse(true);
			}
		}
		else
		{
			// Let parent handle it
			Overlay::OnClick(widget);
		}
	}

	void OverlayMac::UpdateFloppy(BYTE drive)
	{
		const auto& image = GetPC()->GetFloppy(drive).GetImageInfo();

		std::ostringstream os;
		os << (drive ? "External" : "Internal");
		if (image.loaded)
		{
			m_floppyButton[drive]->SetTooltip(image.path.stem().string().c_str());

			os << " ["
				<< image.format
				<< "]";
		}
		else
		{
			m_floppyButton[drive]->SetTooltip("Load floppy image");
		}

		m_floppyButton[drive]->SetText(os.str().c_str());
	}

	bool OverlayMac::Update()
	{
		if (!Overlay::Update())
		{
			return false;
		}

		for (int i = 0; i < 2; ++i)
		{
			if (const auto& floppy = GetPC()->GetFloppy(i); floppy.IsConnected())
			{
				m_floppyButton[i]->SetImage(floppy.IsActive() ? m_floppyActive : m_floppyInactive);
			}
		}

		bool captured = m_pc->GetInputs().IsMouseCaptured();
		if (m_mouseCaptured != captured)
		{
			m_mouseButton->SetImage(captured ? m_mouseCaptureOn : m_mouseCaptureOff);
			m_mouseButton->SetPushed(captured);
			m_mouseCaptured = captured;
		}

		return true;
	}

	void OverlayMac::LoadFloppyDiskImage(BYTE drive)
	{
		if (!GetPC())
		{
			return;
		}

		auto& floppy = GetPC()->GetFloppy(drive);
		if (floppy.IsDiskLoaded())
		{
			return;
		}

		fs::path diskImage;

		if (SelectFile(diskImage, { {"Floppy disk image (*.img)", "*.img"} }))
		{
			floppy.LoadDiskImage(diskImage.string().c_str());
		}
		UpdateFloppy(drive);
	}
}
