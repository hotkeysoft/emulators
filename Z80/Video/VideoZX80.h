#pragma once
#include "Video.h"

namespace video
{
    class VideoZX80 : public Video
    {
    public:
        VideoZX80();

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false);

        virtual const std::string GetID() const override { return "zx80"; }
        virtual void Tick() override;

        virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;

        virtual bool IsEnabled() const override { return true; }

        virtual bool IsVSync() const override { return m_vSync; }
        virtual bool IsHSync() const override { return m_hSync; }
        virtual bool IsDisplayArea() const override { return !IsVSync() && !IsHSync(); }

        void HSync(bool set = true);
        void VSync(bool set);

        void LatchCurrentChar(BYTE ch);

        DWORD GetDefaultBackground() const { return 0xFFD3D3D3; }
        DWORD GetDefaultForeground() const { return 0xFF282828; }

        void SetBackground(DWORD rgb) { m_background = rgb | 0xFF000000; }
        void SetForeground(DWORD rgb) { m_foreground = rgb | 0xFF000000; }

    protected:
        void DrawChar();

        BYTE m_currentChar = 0;
        bool m_invertChar = false;

        // "Colors"
        DWORD m_background = GetDefaultBackground();
        DWORD m_foreground = GetDefaultForeground();

        bool m_vSync = false;
        bool m_hSync = false;

        BYTE m_rowCounter = 0;
    };
}
