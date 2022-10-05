#pragma once

#include <Video/Video.h>
#include <UI/Overlay.h>
#include "../IO/DeviceJoystick.h"
#include "../Computer.h"
#include <CoreUI.h>

namespace ui
{
	// Adds some hysteresis so the light is visible
	class HardDriveLED
	{
	public:
		void Update(bool active);
		bool GetStatus() const { return m_active; }

	private:
		static const int CooldownTime = 50;

		bool m_active = false;
		bool m_lastActive = false;
		int m_cooldown = CooldownTime;
	};

	using NewComputerCallback = void (*)(std::filesystem::path, json& data);

	class OverlayXT : public Overlay
	{
	public:
		OverlayXT();

		virtual bool Init() override;
		virtual void SetPC(emul::ComputerBase* pc) override;
		virtual bool Update() override;

	protected:
		emul::Computer* GetPC() { return (emul::Computer*)m_pc; }

		virtual void OnClick(CoreUI::WidgetRef widget) override;
		virtual void OnClose(CoreUI::WidgetRef widget) override;

		virtual void UpdateTitle(float fps = 0.) override;

		void UpdateSpeed();
		void UpdateFloppy(BYTE drive);
		void UpdateHardDisk(BYTE drive);
		void UpdateTrim();

		void LoadFloppyDiskImage(BYTE drive, bool eject = false);
		void LoadHardDiskImage(BYTE drive);
		void ToggleCPUSpeed();
		void SetTrim();

		void JoystickConfig();

		HardDriveLED m_hardDriveLEDs[2];

		bool m_mouseCaptured = false;

		// UI Elements
		CoreUI::WindowPtr m_joystickConfigWnd;

		CoreUI::ToolbarItemPtr m_floppyButton[2];
		CoreUI::ToolbarItemPtr m_ejectButton[2];
		CoreUI::ToolbarItemPtr m_hddButton[2];
		CoreUI::ToolbarItemPtr m_speedButton;
		CoreUI::ToolbarItemPtr m_joystickButton;
		CoreUI::ToolbarItemPtr m_mouseButton;

		CoreUI::ImageRef m_floppyInactive = nullptr;
		CoreUI::ImageRef m_floppyActive = nullptr;

		CoreUI::ImageRef m_hddInactive = nullptr;
		CoreUI::ImageRef m_hddActive = nullptr;

		CoreUI::ImageRef m_mouseCaptureOff = nullptr;
		CoreUI::ImageRef m_mouseCaptureOn = nullptr;

		CoreUI::LabelPtr m_trimX = nullptr;
		CoreUI::LabelPtr m_trimY = nullptr;

		joy::AxisTrim m_trim;
	};
}
