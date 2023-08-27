#pragma once
#include <Video/Video.h>
#include <CPU/IOConnector.h>

namespace video::mac
{
    class EventHandler
    {
    public:
        virtual void OnHBlankStart() {};
        virtual void OnVBlankStart() {};
    };

    class VideoMac : public Video
    {
    public:
        VideoMac();

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;

        void SetEventHandler(video::mac::EventHandler* handler) { assert(handler); m_events = handler; }

        virtual const std::string GetID() const override { return "mac"; }
        virtual void Tick() override;

        virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;

        virtual bool IsEnabled() const override { return true; }

        bool IsHBlank() const { return m_currX < LEFT_BORDER_PX || m_currX >= RIGHT_BORDER_PX; }
        bool IsVBlank() const { return m_currY < TOP_BORDER || m_currY >= BOTTOM_BORDER; }

        virtual bool IsHSync() const override { return IsHBlank(); }
        virtual bool IsVSync() const override { return IsVBlank(); }
        virtual bool IsDisplayArea() const override { return !IsHBlank() && !IsVBlank(); }

        uint32_t GetX() const { return m_currX - LEFT_BORDER_PX; }
        uint32_t GetY() const { return m_currY - TOP_BORDER; }

        void SetBaseAddress(ADDRESS base) { m_baseAddress = base; }

    protected:
        void Draw();

        // DOT Clock it 15.7MHz, CPU Clock is 7.833MHz. Tick() is called at CPU clock rate
        // Video draws two pixels for each tick
        // Video reads one word for memory every 16 pixels, so every 8 ticks
        //
        // 704 characters per line: 512 (display) + 192 (hblank+vsync)
        // 370 lines (342 + 28 lines vblank+vsync)
        // 704 * 370 = 260480 dots / frame
        // 704 * 370 / 2 = 130240 cpu ticks / frame
        // @7.8336.MHz we get 60.15 frames / second.

        // Number of lines displayed
        static constexpr uint32_t V_DISPLAY = 342;

        // Vertical Blanking
        static constexpr uint32_t V_BLANK = 28; // 14 lines on top, 14 on bottom

        // Total number of lines (including blanking area)
        static constexpr uint32_t V_TOTAL = V_DISPLAY + V_BLANK;

        // Number of pixels per line (displayed)
        static constexpr uint32_t H_DISPLAY_PX = 512;

        // Horizontal blanking
        static constexpr uint32_t H_BLANK_PX = 192; // 96 on each side

        // Total number of characters per line (including blanking area)
        static constexpr uint32_t H_TOTAL_PX = H_DISPLAY_PX + H_BLANK_PX;

        // Borders
        static constexpr uint32_t LEFT_BORDER_PX = H_BLANK_PX / 2;
        static constexpr uint32_t RIGHT_BORDER_PX = H_DISPLAY_PX + LEFT_BORDER_PX;

        static constexpr uint32_t TOP_BORDER = V_BLANK / 2;
        static constexpr uint32_t BOTTOM_BORDER = V_DISPLAY + TOP_BORDER;

        uint32_t m_currX = 0;
        uint32_t m_currY = 0;

        ADDRESS m_baseAddress = 0x61A700;

        // Only valid while IsDisplayArea()
        ADDRESS m_currAddress = 0;

        video::mac::EventHandler* m_events = nullptr;
    };
}
