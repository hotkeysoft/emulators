#pragma once
#include "Video.h"

namespace via
{
    class Device6522PET;
}

namespace video
{
    class VideoPET2001 : public Video
    {
    public:
        VideoPET2001();

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false);
        void SetVIA(via::Device6522PET* via) { m_via = via; }

        virtual const std::string GetID() const override { return "pet2001"; }
        virtual void Tick() override;

        virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;

        virtual bool IsEnabled() const override { return true; }

        // More "IsBlanking", we don't really care about vsync/hsync pulse width
        // This returns true if we're in an horizontal or blanking interval
        // PET doesn't care about hsync, but uses the vertical blanking interval
        // to update video RAM (otherwise we'd get flickering - not emulated)
        virtual bool IsVSync() const override { return m_currY >= V_DISPLAY; }
        virtual bool IsHSync() const override { return m_currX >= H_DISPLAY; }
        virtual bool IsDisplayArea() const override { return !IsVSync() && !IsHSync(); }

    protected:
        via::Device6522PET* m_via = nullptr;
        bool m_graphics = false;

        // I don't know the real timings so this'll have to do for now.
        // DOT Clock it 8MHz, CPU Clock is 1MHz. Tick() is called at CPU clock rate (1MHz)
        // so we draw 8 pixels for each tick.
        // Let's assume 64 characters per line (40 + 34 blank+hsync)
        // and 262 lines (because it's the NTSC number of lines).
        // with 200 lines in display area (25 characters * 8 pixels height + 62 lines blank+vsync)
        // 64 * 262 = 16768 ticks / frame
        // @1MHz we get 59.64 frames / second.

        // Pixels per character
        static const uint32_t CHAR_WIDTH = 8;
        static const uint32_t CHAR_HEIGHT = 8;

        // Number of lines displayed
        static const uint32_t V_DISPLAY = 200;
        // Total number of lines (including borders)
        static const uint32_t V_TOTAL = 262;

        // Number of characters per line (displayed)
        static const uint32_t H_DISPLAY = 40;
        static const uint32_t H_DISPLAY_PX = H_DISPLAY * CHAR_WIDTH;

        // Total number of characters per line (including borders)
        static const uint32_t H_TOTAL = 64;
        static const uint32_t H_TOTAL_PX = H_TOTAL * CHAR_WIDTH;

        // Split borders evenly on both sides
        static const uint32_t LEFT_BORDER = (H_TOTAL - H_DISPLAY) / 2;
        static const uint32_t LEFT_BORDER_PX = LEFT_BORDER * CHAR_WIDTH;
        static const uint32_t TOP_BORDER = (V_TOTAL - V_DISPLAY) / 2;
        static const uint32_t RIGHT_BORDER = (H_DISPLAY + H_TOTAL) / 2;
        static const uint32_t BOTTOM_BORDER = (V_DISPLAY + V_TOTAL) / 2;

        // PET has a purely character display, 40x25 = 1000 characters
        // 1K RAM buffer @ 0x8000
        const ADDRESS CHAR_BASE = 0x8000;
        ADDRESS m_currChar = CHAR_BASE;

        emul::MemoryBlock m_charROM;

        void DrawChar();

        uint32_t m_bgColor = 0xFF000000;
        uint32_t m_fgColor = 0xFFFFFFFF;

        uint32_t m_currX = 0;
        uint32_t m_currY = 0;
        BYTE m_currRow = 0;
    };
}
