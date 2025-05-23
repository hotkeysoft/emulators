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

        void SetAlphaGraph(bool graph) { m_alphaGraph = graph; }
        void SetAlphaSemigraph(bool semigraph) { m_alphaSemigraph = semigraph; }
        void SetIntExt(bool ext) { m_intExt = ext; }
        void SetGM0(bool set) { m_gm0 = set; }
        void SetGM1(bool set) { m_gm1 = set; }
        void SetGM2(bool set) { m_gm2 = set; }
        void SetCSS(bool set) { m_css = set; }
        void SetInverse(bool set) { m_inverse = set; }

    protected:
        bool m_alphaGraph = false;
        bool m_alphaSemigraph = false;
        bool m_intExt = false;
        bool m_gm0 = false;
        bool m_gm1 = false;
        bool m_gm2 = false;
        bool m_css = false;
        bool m_inverse = false;

        void Draw();
        void DrawAlpha4(BYTE currChar);
        void DrawSemigraph4(BYTE currChar);

        // TODO: Update timing info


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
