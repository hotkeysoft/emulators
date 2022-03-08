#pragma once

#include "../Video/Video.h"
#include "../IO/InputEvents.h"
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

	class Overlay : public Logger, public video::Renderer, public events::EventHandler
	{
	public:
		Overlay();

		bool Init(emul::Computer* pc);
		void Show(bool show = true) { m_show = show; }
		bool Update();

		// video::Renderer
		virtual void Render() override;

		// events::EventHangler
		virtual bool HandleEvent(SDL_Event& e) override;

		// TODO: Should not be hardcoded
		static const WORD GetOverlayHeight() { return 62; }

	protected:
		bool m_show = true;

		void OnClick(CoreUI::WidgetRef widget);

		void UpdateSpeed();
		void UpdateSnapshot();
		void UpdateFloppy(BYTE drive);
		void UpdateHardDisk(BYTE drive);
		void UpdateTurbo();
		void UpdateTrim();
		void UpdateTitle(float fps = 0.);

		void LoadFloppyDiskImage(BYTE drive, bool eject = false);
		void LoadHardDiskImage(BYTE drive);
		void ToggleCPUSpeed();
		void ToggleTurbo();
		void SetTrim();

		void JoystickConfig();

		// TODO: Should be in separate class so it can be used by others
		bool MakeSnapshotDirectory(std::filesystem::path& dir);
		bool GetSnapshotBaseDirectory(std::filesystem::path& baseDir);
		bool GetLastSnapshotDirectory(std::filesystem::path& snapshotDir);
		void SaveSnapshot(const std::filesystem::path& snapshotDir);
		void RestoreSnapshot(const std::filesystem::path& snapshotDir);
		void DeleteSnapshot(const std::filesystem::path& snapshotDir);
		std::string GetSnapshotName(const std::filesystem::path& snapshotDir);
		void LoadSnapshotList();
		void LoadLastSnapshot();
		void RemoveSnapshotWindow();
		void ShowSnapshotWindow();
		void AddSnapshotItem(const std::filesystem::path& path, int index, int w, int h);

		std::string m_title = "PC";

		emul::Computer* m_pc = nullptr;
		std::filesystem::path m_snapshotBaseDirectory;
		std::filesystem::path m_lastSnapshotDir;

		HardDriveLED m_hardDriveLEDs[2];

		bool m_turbo = false;

		// UI Elements
		CoreUI::WindowPtr m_mainWnd;
		CoreUI::WindowPtr m_joystickConfigWnd;
		CoreUI::WindowPtr m_snapshotWnd;

		CoreUI::RendererRef m_renderer = nullptr;
		CoreUI::MainWindowRef m_window = nullptr;

		CoreUI::ToolbarItemPtr m_floppyButton[2];
		CoreUI::ToolbarItemPtr m_ejectButton[2];
		CoreUI::ToolbarItemPtr m_hddButton[2];
		CoreUI::ToolbarItemPtr m_speedButton;
		CoreUI::ToolbarItemPtr m_turboButton;
		CoreUI::ToolbarItemPtr m_loadSnapshotButton;
		CoreUI::ToolbarItemPtr m_joystickButton;

		CoreUI::ImageRef m_floppyInactive = nullptr;
		CoreUI::ImageRef m_floppyActive = nullptr;

		CoreUI::ImageRef m_hddInactive = nullptr;
		CoreUI::ImageRef m_hddActive = nullptr;

		CoreUI::ImageRef m_turboOff = nullptr;
		CoreUI::ImageRef m_turboOn = nullptr;

		CoreUI::LabelPtr m_trimX = nullptr;
		CoreUI::LabelPtr m_trimY = nullptr;

		joy::AxisTrim m_trim;

		std::set<std::filesystem::path> m_snapshots;
	};

}