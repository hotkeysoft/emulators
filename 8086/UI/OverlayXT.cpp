#include "stdafx.h"

#include <UI/MainWindow.h>
#include "OverlayXT.h"
#include <UI/TimeFormatter.h>
#include <UI/SnapshotInfo.h>
#include <UI/SnapshotWidget.h>
#include <Config.h>
#include "../Storage/DeviceFloppy.h"
#include "../Storage/DeviceHardDrive.h"

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

#include <commdlg.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_syswm.h>

namespace fs = std::filesystem;

using cfg::CONFIG;
using namespace CoreUI;

namespace ui
{
	HWND GetHWND()
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(MAINWND().GetWindow(), &wmInfo);
		return wmInfo.info.win.window;
	}

	bool SelectFile(fs::path& path, HWND parent)
	{
		OPENFILENAMEA ofn;
		char szFile[1024];

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = parent;
		ofn.lpstrFile = szFile;
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
		// use the contents of szFile to initialize itself.
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All\0*.*\0Floppy Image\0*.IMG\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		// Display the Open dialog box.

		if (!GetOpenFileNameA(&ofn))
		{
			return false;
		}
		path = szFile;
		return true;
	}

	// TODO: The actual refresh interval is proportional
	// to the emulated CPU spees
	void HardDriveLED::Update(bool active)
	{
		m_active = active;
		if (active)
		{
			m_cooldown = HardDriveLED::CooldownTime;
			m_lastActive = true;
		}
		else if (m_lastActive) // 1->0
		{
			m_active = true;
			--m_cooldown;
			if (m_cooldown == 0)
			{
				m_active = false;
				m_lastActive = false;
			}
		}
	}

	OverlayXT::OverlayXT()
	{
	}

	bool OverlayXT::Init()
	{
		if (!Overlay::Init())
			return false;

		m_floppyInactive = RES().FindImage("overlay16", 0);
		m_floppyActive = RES().FindImage("overlay16", 4);
		m_hddInactive = RES().FindImage("overlay16", 1);
		m_hddActive = RES().FindImage("overlay16", 5);
		m_mouseCaptureOff = RES().FindImage("overlay16", 16);
		m_mouseCaptureOn = RES().FindImage("overlay16", 17);
		return true;
	}

	void OverlayXT::SetPC(emul::Computer* pc)
	{
		Overlay::SetPC(pc);

		m_rebootButton->SetTooltip("Click for Soft Reboot (CTRL+ATL+DEL)\nShift-Click for Hard Reboot");

		GetToolbar()->AddSeparator();

		// Toolbar section: Floppy drives
		if (m_pc->GetFloppy())
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

		// Toolbar section: Hard disks
		if (m_pc->GetHardDrive())
		{
			m_hddButton[0] = GetToolbar()->AddToolbarItem("hdd0", m_hddInactive, "C:");
			UpdateHardDisk(0);

			m_hddButton[1] = GetToolbar()->AddToolbarItem("hdd1", m_hddInactive, "D:");
			UpdateHardDisk(1);

			GetToolbar()->AddSeparator();
		}
		else
		{
			m_hddButton[0] = nullptr;
			m_hddButton[1] = nullptr;
		}

		m_turboButton->SetText(nullptr);

		// Toolbar section: Speed
		m_speedButton = GetToolbar()->AddToolbarItem("speed", RES().FindImage("overlay16", 6), " 0.00 MHz");
		m_speedButton->SetTooltip("Toggle CPU Speed");
		UpdateSpeed();

		// Toolbar section: Joystick
		if (m_pc->GetInputs().GetJoystick())
		{
			GetToolbar()->AddSeparator();

			m_joystickButton = GetToolbar()->AddToolbarItem("joystick", RES().FindImage("overlay16", 2));
			m_joystickButton->SetTooltip("Joystick Configuration");
		}
		else
		{
			m_joystickButton = nullptr;
		}

		if (m_pc->GetInputs().GetMouse())
		{
			GetToolbar()->AddSeparator();

			m_mouseButton = GetToolbar()->AddToolbarItem("mouse", m_mouseCaptureOff);
			m_mouseButton->SetTooltip("Toggle Mouse Capture\nKeyboard: [Scroll Lock] key");
		}
		else
		{
			m_mouseButton = nullptr;
		}
	}

	void OverlayXT::OnClick(CoreUI::WidgetRef widget)
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
		else if (id == "hdd0")
		{
			LoadHardDiskImage(0);
		}
		else if (id == "hdd1")
		{
			LoadHardDiskImage(1);
		}
		else if (id == "speed")
		{
			ToggleCPUSpeed();
		}
		else if (id == "reboot") // Override parent
		{
			// Shift+click = hard reboot
			m_pc->Reboot(SDL_GetModState() & KMOD_SHIFT);
		}
		else if (id == "joystick")
		{
			JoystickConfig();
		}
		else if (id == "mouse")
		{
			if (m_pc)
			{
				m_pc->GetInputs().CaptureMouse(true);
			}
		}
		else if (id == "trimX-")
		{
			--m_trim.x;
			SetTrim();
		}
		else if (id == "trimX--")
		{
			m_trim.x -= 5;
			SetTrim();
		}
		else if (id == "trimX+")
		{
			++m_trim.x;
			SetTrim();
		}
		else if (id == "trimX++")
		{
			m_trim.x += 5;
			SetTrim();
		}
		else if (id == "trimY-")
		{
			--m_trim.y;
			SetTrim();
		}
		else if (id == "trimY--")
		{
			m_trim.y -= 5;
			SetTrim();
		}
		else if (id == "trimY+")
		{
			++m_trim.y;
			SetTrim();
		}
		else if (id == "trimY++")
		{
			m_trim.y += 5;
			SetTrim();
		}
		else if (id == "resetX")
		{
			m_trim.x = 0;
			SetTrim();
		}
		else if (id == "resetY")
		{
			m_trim.y = 0;
			SetTrim();
		}
		else
		{
			// Let parent handle it
			Overlay::OnClick(widget);
		}
	}

	void OverlayXT::OnClose(CoreUI::WidgetRef widget)
	{
		if (widget->GetId() == "joystick")
		{
			m_joystickButton->SetPushed(false);
		}
		else
		{
			Overlay::OnClose(widget);
		}
	}

	void OverlayXT::UpdateSpeed()
	{
		if (!m_pc)
		{
			return;
		}

		static char speedStr[32];
		emul::CPUSpeed speed = m_pc->GetCPUSpeed();
		sprintf(speedStr, "%.2fMHz", speed.GetSpeed() / 1000000.0f);
		m_speedButton->SetText(speedStr);
	}

	void OverlayXT::UpdateFloppy(BYTE drive)
	{
		const auto& image = m_pc->GetFloppy()->GetImageInfo(drive);

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

	void OverlayXT::UpdateHardDisk(BYTE drive)
	{
		const auto& image = m_pc->GetHardDrive()->GetImageInfo(drive);

		std::ostringstream os;
		os << (char)('C' + drive) << ':';
		std::string label = "[Empty]";
		if (image.loaded)
		{
			m_hddButton[drive]->SetTooltip(image.path.stem().string().c_str());

			os << " ["
				<< std::fixed << std::setprecision(1) << (image.geometry.GetImageSize() / 1048576.0)
				<< "MB]";
		}
		else
		{
			m_hddButton[drive]->SetTooltip("Load hard disk image");
		}

		m_hddButton[drive]->SetText(os.str().c_str());
	}

	void OverlayXT::UpdateTrim()
	{
		m_trim = m_pc->GetInputs().GetJoystick()->GetAxisTrim();
		{
			std::ostringstream os;
			os << m_trim.x;
			m_trimX->SetText(os.str().c_str());
		}

		{
			std::ostringstream os;
			os << m_trim.y;
			m_trimY->SetText(os.str().c_str());
		}
	}

	void OverlayXT::SetTrim()
	{
		m_pc->GetInputs().GetJoystick()->SetAxisTrim({ m_trim.x, m_trim.y });
		UpdateTrim();
	}

	bool OverlayXT::Update()
	{
		if (!Overlay::Update())
		{
			return false;
		}

		if (m_pc->GetFloppy())
		{
			for (int i = 0; i < 2; ++i)
			{
				m_floppyButton[i]->SetImage(m_pc->GetFloppy()->IsActive(i) ? m_floppyActive : m_floppyInactive);
			}
		}

		if (m_pc->GetHardDrive())
		{
			for (int i = 0; i < 2; ++i)
			{
				m_hardDriveLEDs[i].Update(m_pc->GetHardDrive()->IsActive(i));
				m_hddButton[i]->SetImage(m_hardDriveLEDs[i].GetStatus() ? m_hddActive : m_hddInactive);
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

	void OverlayXT::UpdateTitle(float fps)
	{
		if (!m_pc)
		{
			return;
		}

		std::ostringstream os;
		os << m_pc->GetName();

		// Display video mode only if non-trivial (e.g skip pcjr)
		const auto& videoModes = m_pc->GetVideoModes();
		if (videoModes.size() > 1)
		{
			os << " [" << m_pc->GetVideo().GetDisplayName() << "]";
		}

		if (fps > 0.)
		{
			os << " [" << std::fixed << std::setprecision(1) << fps << " fps]";
		}

		m_mainWnd->SetText(os.str().c_str());
	}


	void OverlayXT::LoadFloppyDiskImage(BYTE drive, bool eject)
	{
		if (!m_pc || !m_pc->GetFloppy())
		{
			return;
		}

		fs::path diskImage;

		if (eject)
		{
			m_pc->GetFloppy()->ClearDiskImage(drive);
		}
		else if (SelectFile(diskImage, GetHWND()))
		{
			m_pc->GetFloppy()->LoadDiskImage(drive, diskImage.string().c_str());
		}
		UpdateFloppy(drive);
	}

	void OverlayXT::LoadHardDiskImage(BYTE drive)
	{
		if (!m_pc || !m_pc->GetHardDrive())
		{
			return;
		}

		fs::path diskImage;
		if (SelectFile(diskImage, GetHWND()))
		{
			BYTE currImageType = m_pc->GetHardDrive()->GetImageInfo(drive).type;

			// Assume same hdd type for now to simplify things.
			// Incompatible images will be rejected
			m_pc->GetHardDrive()->LoadDiskImage(drive, currImageType, diskImage.string().c_str());
		}
		UpdateHardDisk(drive);
	}

	void OverlayXT::ToggleCPUSpeed()
	{
		const emul::CPUSpeed currSpeed = m_pc->GetCPUSpeed();
		const emul::Computer::CPUSpeeds& speeds = m_pc->GetCPUSpeeds();
		emul::Computer::CPUSpeeds::const_iterator currIt = speeds.find(currSpeed);
		if (currIt == speeds.end())
		{
			currIt = speeds.begin();
		}
		else
		{
			++currIt;
			if (currIt == speeds.end())
			{
				currIt = speeds.begin();
			}
		}
		m_pc->SetCPUSpeed(*currIt);
		UpdateSpeed();
	}

	void OverlayXT::JoystickConfig()
	{
		if (m_joystickConfigWnd)
		{
			bool isVisible = m_joystickConfigWnd->GetShowState() & WindowState::WST_VISIBLE;

			m_joystickConfigWnd->Show(isVisible ? false : true);
			m_joystickButton->SetPushed(!isVisible);
			m_joystickConfigWnd->SetActive();
			return;
		}

		m_joystickButton->SetPushed(true);

		const int width = 278;
		const int height = 90;
		const int line0y = 4;
		const int line1y = 28;
		const int lineh = 24;

		SDL_Renderer* ren = MAINWND().GetRenderer();

		Rect r = WINMGR().GetWindowSize();
		r.x = r.w - width;
		r.y = r.h - (GetOverlayHeight() + height);
		r.w = width;
		r.h = height;
		m_joystickConfigWnd = WINMGR().AddWindow("joystick", r, WIN_CLOSE | WIN_DIALOG | WIN_CANMOVE | WIN_NOSCROLL);
		m_joystickConfigWnd->SetText("Joystick");
		m_joystickConfigWnd->SetActive();

		m_joystickConfigWnd->AddControl(CoreUI::Label::CreateSingle("lx", ren, Rect(0, line0y, 50, lineh), "Trim X"));
		m_joystickConfigWnd->AddControl(CoreUI::Label::CreateSingle("ly", ren, Rect(0, line1y, 50, lineh), "Trim Y"));

		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimX--", ren, Rect(50, line0y, 28, lineh), "<<"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimY--", ren, Rect(50, line1y, 28, lineh), "<<"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimX-", ren, Rect(76, line0y, 24, lineh), "<"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimY-", ren, Rect(76, line1y, 24, lineh), "<"));

		m_trimX = CoreUI::Label::CreateSingle("trimX", ren, Rect(110, line0y, 28, lineh), "0", nullptr, CoreUI::Label::TEXT_H_CENTER);
		m_joystickConfigWnd->AddControl(m_trimX);

		m_trimY = CoreUI::Label::CreateSingle("trimY", ren, Rect(110, line1y, 28, lineh), "0", nullptr, CoreUI::Label::TEXT_H_CENTER);
		m_joystickConfigWnd->AddControl(m_trimY);

		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimX+", ren, Rect(146, line0y, 24, lineh), ">"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimY+", ren, Rect(146, line1y, 24, lineh), ">"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimX++", ren, Rect(170, line0y, 28, lineh), ">>"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimY++", ren, Rect(170, line1y, 28, lineh), ">>"));

		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("resetX", ren, Rect(206, line0y, 60, lineh), "Reset"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("resetY", ren, Rect(206, line1y, 60, lineh), "Reset"));

		UpdateTrim();
	}
}
