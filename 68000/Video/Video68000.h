#pragma once
#include <Video/Video.h>

namespace video
{
    class Video68000 : public Video
    {
    public:
        Video68000();

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false);

        virtual const std::string GetID() const override { return "68000"; }
        virtual void Tick() override;

        virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;

        virtual bool IsEnabled() const override { return true; }

        virtual bool IsVSync() const override { return false; }
        virtual bool IsHSync() const override { return false; }
        virtual bool IsDisplayArea() const override { return !IsVSync() && !IsHSync(); }

    protected:
        void DrawChar();
    };
}
