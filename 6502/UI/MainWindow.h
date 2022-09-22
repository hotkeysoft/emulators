#pragma once

struct SDL_Window;
struct SDL_Renderer;

namespace ui
{
	struct WindowSize 
	{
		int w = 0;
		int h = 0;
	};

	// Singleton
	class MainWindow : public Logger
	{
	public:
		MainWindow(const MainWindow&) = delete;
		MainWindow& operator=(const MainWindow&) = delete;
		MainWindow(MainWindow&&) = delete;
		MainWindow& operator=(MainWindow&&) = delete;

		static MainWindow& Get();

		bool Init();

		SDL_Window* GetWindow() const { return m_sdlWindow; }
		SDL_Renderer* GetRenderer() const { return m_sdlRenderer; }

		const WindowSize& GetSize() const { return m_size; }
		void SetSize(const WindowSize size) { m_size = size; }

	protected:
		MainWindow();

		WindowSize m_size = { 0, 0 };

		// SDL
		SDL_Window* m_sdlWindow = nullptr;
		SDL_Renderer* m_sdlRenderer = nullptr;
	};

	constexpr auto MAINWND = &MainWindow::Get;
}

