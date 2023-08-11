#pragma once
#include <Video/Video.h>
#include <CPU/IOConnector.h>

namespace video
{
    class VideoThomson : public Video, public emul::IOConnector
    {
    public:
        VideoThomson();

        virtual void Init(emul::MemoryBlock* pixelRAM, emul::MemoryBlock* attributeRAM);

        virtual const std::string GetID() const override { return "thomson"; }
        virtual void Tick() override;

        virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;

        void SetBorderColor(BYTE borderRGBP) { m_borderColor = m_palette[borderRGBP & 15]; }

        virtual bool IsEnabled() const override { return true; }

        bool IsHBlank() const { return m_currX < LEFT_BORDER || m_currX >= RIGHT_BORDER; }
        bool IsVBlank() const { return m_currY < TOP_BORDER || m_currY >= BOTTOM_BORDER; }

        virtual bool IsHSync() const override { return m_currX >= H_SYNC_START; }
        virtual bool IsVSync() const override { return m_currY >= V_SYNC_START; }
        virtual bool IsDisplayArea() const override { return !IsHBlank() && !IsVBlank(); }

        uint32_t GetX() const { return m_currX - LEFT_BORDER; }
        uint32_t GetY() const { return m_currY - TOP_BORDER; }

    protected:
        void Draw();

        // Gate Array Mem IO
        BYTE ReadSCRCLKhigh();  // A7E4
        BYTE ReadSCRCLKlow();   // A7E5
        BYTE ReadLineCounter(); // A7E6
        BYTE ReadINITN();       // A7E7

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
        static const uint32_t V_DISPLAY = 200;
        // Total number of lines (including borders)
        static const uint32_t V_TOTAL = 312;

        // Vertical Blanking
        // TODO: Check values, those are random
        static const uint32_t V_BLANK = 100; // 50 lines on top, 50 on bottom
        static const uint32_t V_SYNC = 12;
        static const uint32_t V_SYNC_START = V_DISPLAY + V_BLANK;

        // Number of characters per line (displayed)
        static const uint32_t H_DISPLAY = 40;
        static const uint32_t H_DISPLAY_PX = H_DISPLAY * CHAR_WIDTH;

        // Total number of characters per line (including borders)
        static const uint32_t H_TOTAL = 64;
        static const uint32_t H_TOTAL_PX = H_TOTAL * CHAR_WIDTH;

        // Horizontal blanking
        static const uint32_t H_BLANK = 14; // 7 on each side
        static const uint32_t H_SYNC = 10;
        static const uint32_t H_SYNC_START = H_DISPLAY + H_BLANK;

        // Borders
        static const uint32_t LEFT_BORDER = H_BLANK / 2;
        static const uint32_t LEFT_BORDER_PX = LEFT_BORDER * CHAR_WIDTH;
        static const uint32_t RIGHT_BORDER = H_DISPLAY + LEFT_BORDER;

        static const uint32_t TOP_BORDER = V_BLANK / 2;
        static const uint32_t BOTTOM_BORDER = V_DISPLAY + TOP_BORDER;

        uint32_t m_currX = 0;
        uint32_t m_currY = 0;

        // Only valid while IsDisplayArea()
        ADDRESS m_currChar = 0;

        emul::MemoryBlock* m_pixelRAM = nullptr;
        emul::MemoryBlock* m_attributeRAM = nullptr;

        uint32_t m_borderColor = 0;

        const uint32_t m_palette[16] = {
            0xFF000000, 0xFFFF0000, 0xFF00FF00, 0xFFFFFF00,
            0xFF0000FF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF,
            0xFFBBBBBB, 0xFFDD7777, 0xFF77DD77, 0xFFDDDD77,
            0xFF7777DD, 0xFFDD77EE, 0xFFBBFFFF, 0xFFEEBB00,
        };
    };
}
