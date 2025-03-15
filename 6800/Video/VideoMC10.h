#pragma once
#include <Video/Video.h>

namespace video
{
    class VideoMC10: public Video
    {
    public:
        VideoMC10();

        virtual void Init(emul::Memory* memory, const char* charROM);

        virtual const std::string GetID() const override { return "mc10"; }
        virtual void Tick() override;

        virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;


        virtual bool IsEnabled() const override { return true; }

        bool IsHBlank() const { return m_currX < LEFT_BORDER || m_currX >= RIGHT_BORDER; }
        bool IsVBlank() const { return m_currY < TOP_BORDER || m_currY >= BOTTOM_BORDER; }

        virtual bool IsHSync() const override { return m_currX >= H_SYNC_START; }
        virtual bool IsVSync() const override { return m_currY >= V_SYNC_START; }
        virtual bool IsDisplayArea() const override { return !IsHBlank() && !IsVBlank(); }

        uint32_t GetX() const { return m_currX - LEFT_BORDER; }
        uint32_t GetY() const { return m_currY - TOP_BORDER; }

        virtual uint32_t GetBackgroundColor() const override { return m_borderColor; }

        void SetCSS(bool set) { LogPrintf(LOG_DEBUG, "SetCSS(%d)", set); }
        void SetAlphaGraph(bool set) { LogPrintf(LOG_DEBUG, "SetAlphaGraph(%d)", set); }
        void SetGM0(bool set) { LogPrintf(LOG_DEBUG, "SetGM0(%d)", set); }
        void SetGM1(bool set) { LogPrintf(LOG_DEBUG, "SetGM1(%d)", set); }
        void SetGM2(bool set) { LogPrintf(LOG_DEBUG, "SetGM2(%d)", set); }
        void SetIntExt(bool set) { LogPrintf(LOG_DEBUG, "SetIntExt(%d)", set); }

    protected:
        void Draw();

        // TODO: UPDATE
        //
        // DOT Clock it 8MHz, CPU Clock is 1MHz. Tick() is called at CPU clock rate (1MHz)
        // so we draw 8 pixels for each tick.
        // 64 characters per line: 40(display) + 14(blank) + 20(hsync)
        // 312 lines (PAL)
        // with 200 lines in display area (25 characters * 8 pixels height) 112 lines blank+vsync)
        // 64 * 312 = 19968 ticks / frame
        // @1MHz we get 50.08 frames / second.

        // Pixels per character
        static const uint32_t CHAR_WIDTH = 8;

        // Number of lines displayed
        static const uint32_t V_DISPLAY = 192;
        // Total number of lines (including borders + blank)
        static const uint32_t V_TOTAL = 262;

        // Vertical Blanking
        static const uint32_t V_BLANK = 25 + 26; // 25 top, 26 bottom
        static const uint32_t V_SYNC = 13 + 6; // V Blanking + V Retrace
        static const uint32_t V_SYNC_START = V_DISPLAY + V_BLANK;

        // Number of characters per line (displayed)
        static const uint32_t H_DISPLAY = 32;
        static const uint32_t H_DISPLAY_PX = H_DISPLAY * CHAR_WIDTH;

        // Total number of characters per line (including borders)
        static const uint32_t H_TOTAL = 48;
        static const uint32_t H_TOTAL_PX = H_TOTAL * CHAR_WIDTH;

        // Horizontal blanking
        static const uint32_t H_BLANK = 10;
        static const uint32_t H_SYNC = 6;
        static const uint32_t H_SYNC_START = H_DISPLAY + H_BLANK;

        // Borders
        static const uint32_t LEFT_BORDER = H_BLANK / 2;
        static const uint32_t LEFT_BORDER_PX = LEFT_BORDER * CHAR_WIDTH;
        static const uint32_t RIGHT_BORDER = H_DISPLAY + LEFT_BORDER;
        static const uint32_t RIGHT_BORDER_PX = RIGHT_BORDER * CHAR_WIDTH;

        static const uint32_t TOP_BORDER = 25;
        static const uint32_t BOTTOM_BORDER = V_DISPLAY + TOP_BORDER;

        uint32_t m_currX = 0;
        uint32_t m_currY = 0;

        // Only valid while IsDisplayArea()
        ADDRESS m_currChar = 0;

        uint32_t m_borderColor = 0;

        emul::MemoryBlock m_charROM;
    };
}
