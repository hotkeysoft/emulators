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

        void HSync();
        void VSync(bool set);

        void LatchCurrentChar(BYTE ch);

    protected:
        void DrawChar();

        BYTE m_currentChar = 0;
        bool m_invert = false;

        bool m_vSync = false;

        bool m_hSync = false;
        BYTE m_hSyncCounter = 0;

        BYTE m_rowCounter = 0;
    };
}
