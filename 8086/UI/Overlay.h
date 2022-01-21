#pragma once

#include "../Common/Logger.h"
#include "../Video/Video.h"
#include "../IO/InputEvents.h"
#include "../Computer.h"
#include <Common.h>

#include <filesystem>

namespace ui
{
	// Adds some hysteresis so the light is visible
	class HardDriveLED
	{
	public:
		void Update(bool active);
		bool GetStatus() const { return m_active; }

	private:
		static const int CooldownTime = 100000;

		bool m_active = false;
		bool m_lastActive = false;
		int m_cooldown = CooldownTime;
	};

	class Overlay : public Logger, public video::Renderer, public events::EventHandler
	{
	public:
		Overlay();

		bool Init(emul::Computer* pc);
		bool Update();

		// video::Renderer
		virtual void Render() override;

		// events::EventHangler
		virtual bool HandleEvent(SDL_Event& e) override;

	protected:
		void OnClick(CoreUI::WidgetRef widget);

		void UpdateSpeed();
		void UpdateSnapshot();
		void UpdateFloppy(BYTE drive, const char* letter);
		void UpdateHardDisk(BYTE drive, const char* letter);
		void UpdateTurbo();

		void LoadDiskImage(BYTE drive, const char* str, bool eject = false);
		void ToggleCPUSpeed();
		void ToggleTurbo();

		// TODO: Should be in separate class so it can be used by others
		bool MakeSnapshotDirectory(std::filesystem::path& dir);
		bool GetSnapshotBaseDirectory(std::filesystem::path& baseDir);
		bool GetLastSnapshotDirectory(std::filesystem::path& snapshotDir);
		void SaveSnapshot(const std::filesystem::path& snapshotDir);
		void RestoreSnapshot(const std::filesystem::path& snapshotDir);
		std::string GetSnapshotName(const std::filesystem::path& snapshotDir);

		std::string m_title = "PC";

		emul::Computer* m_pc = nullptr;
		std::filesystem::path m_snapshotBaseDirectory;
		std::filesystem::path m_lastSnapshotDir;

		HardDriveLED m_hardDriveLEDs[2];

		bool m_turbo = false;

		// UI Elements
		CoreUI::WindowPtr m_mainWnd;

		CoreUI::RendererRef m_renderer = nullptr;
		CoreUI::MainWindowRef m_window = nullptr;

		CoreUI::ToolbarItemPtr m_floppyButton[2];
		CoreUI::ToolbarItemPtr m_ejectButton[2];
		CoreUI::ToolbarItemPtr m_hddButton[2];

		CoreUI::ToolbarItemPtr m_speedButton;
		CoreUI::ToolbarItemPtr m_snapshot;

		CoreUI::ToolbarItemPtr m_turboButton;

		CoreUI::ImageRef m_floppyInactive = nullptr;
		CoreUI::ImageRef m_floppyActive = nullptr;

		CoreUI::ImageRef m_hddInactive = nullptr;
		CoreUI::ImageRef m_hddActive = nullptr;
	};

}