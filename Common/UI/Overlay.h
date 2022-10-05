#pragma once

#include <Video/Video.h>
#include <IO/InputEventHandler.h>
#include <Computer.h>
#include <CoreUI.h>

namespace ui
{
	using NewComputerCallback = void (*)(std::filesystem::path, json& data);

	class Overlay : public Logger, public video::Renderer, public events::EventHandler
	{
	public:
		Overlay();

		virtual bool Init();
		virtual void SetPC(emul::Computer* pc);
		virtual void Show(bool show = true) { m_show = show; }
		virtual bool Update();

		// When a snapshot is not compatible with the current pc
		// we need to rip it out and create the correct one.
		// This needs to be done in the main main loop at the right time.
		// (The restore snapshot event is called from an event handler deep in the
		// Computer object, so it can't delete itself)
		void SetNewComputerCallback(NewComputerCallback callback) { m_callback = callback; }

		// video::Renderer
		virtual void Render() override;

		// events::EventHandler
		virtual bool HandleEvent(SDL_Event& e) override;

		// TODO: Should not be hardcoded
		static const WORD GetOverlayHeight() { return 62; }

	protected:
		CoreUI::ToolbarPtr GetToolbar() { return m_toolbar; }

		virtual void OnClick(CoreUI::WidgetRef widget);
		virtual void OnClose(CoreUI::WidgetRef widget);

		virtual void UpdateSnapshot();
		virtual void UpdateTurbo();
		virtual void UpdateTitle(float fps = 0.);

		virtual void ToggleTurbo();

		// TODO: Should be in separate class so it can be used by others
		bool MakeSnapshotDirectory(std::filesystem::path& dir);
		bool GetSnapshotBaseDirectory(std::filesystem::path& baseDir);
		bool GetLastSnapshotDirectory(std::filesystem::path& snapshotDir);
		void SaveComputerData(const std::filesystem::path& snapshotDir);
		void SaveSnapshotInfo(const std::filesystem::path& snapshotDir);
		void SaveConfigFile(const std::filesystem::path& snapshotDir);
		void SaveSnapshot(const std::filesystem::path& snapshotDir);
		void RestoreSnapshot(const std::filesystem::path& snapshotDir);
		void DeleteSnapshot(const std::filesystem::path& snapshotDir);
		std::string GetSnapshotName(const std::filesystem::path& snapshotDir);
		void LoadSnapshotList();
		void LoadLastSnapshot();
		void RemoveSnapshotWindow();
		void ShowSnapshotWindow();
		void AddSnapshotItem(const std::filesystem::path& path, int index, int w, int h);

		emul::Computer* m_pc = nullptr;
		std::filesystem::path m_snapshotBaseDirectory;
		std::filesystem::path m_lastSnapshotDir;

		bool m_turbo = false;

		// UI Elements
		CoreUI::WindowPtr m_mainWnd;
		CoreUI::WindowPtr m_snapshotWnd;

		// Common buttons
		CoreUI::ToolbarItemPtr m_rebootButton;
		CoreUI::ToolbarItemPtr m_turboButton;
		CoreUI::ToolbarItemPtr m_loadSnapshotButton;

		CoreUI::ImageRef m_turboOff = nullptr;
		CoreUI::ImageRef m_turboOn = nullptr;

		std::set<std::filesystem::path> m_snapshots;
		NewComputerCallback m_callback = nullptr;

	private:
		bool m_show = true;

		CoreUI::ToolbarPtr m_toolbar;
	};

}