#pragma once
#include <Video/Video.h>

namespace via
{
    class Device6522PET;
}

namespace video
{
    class VideoVIC : public Video
    {
    public:
        VideoVIC();

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false);

        virtual const std::string GetID() const override { return "vic"; }
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
        bool m_graphics = false;

        // TODO: TEMP until VIC implementation

        // Pixels per character
        static const uint32_t CHAR_WIDTH = 8;
        static const uint32_t CHAR_HEIGHT = 8;

        // Number of lines displayed
        static const uint32_t V_DISPLAY = 184;
        // Total number of lines (including borders)
        static const uint32_t V_TOTAL = 262;

        // Number of characters per line (displayed)
        static const uint32_t H_DISPLAY = 22;
        static const uint32_t H_DISPLAY_PX = H_DISPLAY * CHAR_WIDTH;

        // Total number of characters per line (including borders)
        static const uint32_t H_TOTAL = 32;
        static const uint32_t H_TOTAL_PX = H_TOTAL * CHAR_WIDTH;

        // Split borders evenly on both sides
        static const uint32_t LEFT_BORDER = (H_TOTAL - H_DISPLAY) / 2;
        static const uint32_t LEFT_BORDER_PX = LEFT_BORDER * CHAR_WIDTH;
        static const uint32_t TOP_BORDER = (V_TOTAL - V_DISPLAY) / 2;
        static const uint32_t RIGHT_BORDER = (H_DISPLAY + H_TOTAL) / 2;
        static const uint32_t BOTTOM_BORDER = (V_DISPLAY + V_TOTAL) / 2;

        // PET has a purely character display, 40x25 = 1000 characters
        // 1K RAM buffer @ 0x8000
        const ADDRESS CHAR_BASE = 0x1E00;
        const ADDRESS CHARROM_BASE = 0x8000;
        ADDRESS m_currChar = CHAR_BASE;

        void DrawChar();

        uint32_t m_bgColor = 0xFF000000;
        uint32_t m_fgColor = 0xFFFFFFFF;

        uint32_t m_currX = 0;
        uint32_t m_currY = 0;
        BYTE m_currRow = 0;
    };
}
