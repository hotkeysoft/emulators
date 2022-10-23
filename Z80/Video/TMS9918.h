#pragma once
#include <Video/Video.h>
#include <Serializable.h>
#include <CPU/MemoryBlock.h>

namespace video::vdp
{
    enum class VideoMode { GRAPH_1, GRAPH_2, MULTICOLOR, TEXT };

#pragma pack(push, 1)
    struct Sprite
    {
    public:
        // Helpers
        int GetY() const { return (yPos >= 0xE1) ? ((int8_t)(yPos)) : yPos; }
        int GetX() const { return hPos - (IsLeftOffset() ? 32 : 0); }
        bool IsLeftOffset() const { return emul::GetMSB(color); }
        bool IsLast() const { return yPos == 0xD0; }
        BYTE GetName() const { return name; }
        BYTE GetColor() const { return color & 0x0F; }
        bool IsVisible(int y) const {
            const int ymin = GetY();
            const int ymax = ymin + size;
            return (y >= ymin && y < ymax);
        }

        static int GetSize() { return size; }
        static void SetSize(int s) { size = s; }

        static BYTE GetNameMask() { return nameMask; }
        static void SetNameMask(BYTE mask) { nameMask = mask; }

    protected:
        // Raw data
        BYTE yPos;
        BYTE hPos;
        BYTE name;
        BYTE color;

        static int size;
        static BYTE nameMask;
    };
#pragma pack(pop)

    class TMS9918 : public Logger, public emul::Serializable
    {
    public:
        TMS9918();

        void Reset();
        void Init(video::Video* video);

        BYTE ReadStatus();
        void Write(BYTE value);

        BYTE ReadVRAMData();
        void WriteVRAMData(BYTE value);

        void Tick();

        bool IsHDisplay() const { return m_currX >= 0 && m_currX < H_DISPLAY; }
        bool IsVDisplay() const { return m_currY >= 0 && m_currY < V_DISPLAY; }
        bool IsDisplay() const { return IsVDisplay() && IsHDisplay(); }

        bool IsVSync() const { return m_currY >= (BOTTOM_BORDER - V_SYNC); }

        bool IsEnabled() const { return m_config.enable; }

        bool IsInterrupt() const { return m_config.interruptEnabled && m_status.interrupt; };

        constexpr SDL_Rect GetDisplayRect() const {
            return SDL_Rect{ LEFT_BORDER, TOP_BORDER, H_DISPLAY, V_DISPLAY };
        };

        // emul::Serializable
        virtual void Serialize(json& to);
        virtual void Deserialize(const json& from);

    protected:
        static const int H_DISPLAY = 256;
        static const int H_TOTAL = 342;
        static const int H_SYNC = H_TOTAL - 26;

        static const int LEFT_BORDER = 37;
        static const int RIGHT_BORDER = H_DISPLAY + LEFT_BORDER;

        static const int V_DISPLAY = 192;
        static const int V_TOTAL = 262;
        static const int V_SYNC = 3;

        static const int TOP_BORDER = 40;
        static const int BOTTOM_BORDER = TOP_BORDER + V_DISPLAY;

        void WriteRegister(BYTE reg);

        video::Video* m_video = nullptr;
        emul::MemoryBlock m_vram;

        // TODO: Adjust with vram size
        const WORD m_addressMask = 0x3FFF;

        bool m_dataFlipFlop = false;
        BYTE m_tempData = 0;

        WORD m_currReadAddress = 0;
        WORD m_currWriteAddress = 0;

        struct Config
        {
            // Mode Select
            bool m1 = false;
            bool m2 = false;
            bool m3 = false;

            bool vram16k = false;
            bool enable = false;
            bool interruptEnabled = false;
            bool sprites16x16 = false;
            bool sprites2x = false;
        } m_config;

        // Computes m_mode from m1,m2,m3
        void UpdateMode();
        VideoMode m_mode = VideoMode::GRAPH_1;

        struct Status
        {
            bool interrupt = false;
            bool coincidence = false;
            bool fifthSpriteFlag = false;
            BYTE fifthSpriteName = 0;

            void SetFifthSpriteFlag(BYTE id)
            {
                if (!fifthSpriteFlag)
                {
                    fifthSpriteFlag = true;
                    fifthSpriteName = id;
                }
            }
            void Reset();
            BYTE Get();
        } m_status;

        // Table base addresses
        struct Tables
        {
            // Raw values
            WORD rawName = 0;
            WORD rawColor = 0;
            WORD rawPattern = 0;
            WORD rawSpriteAttr = 0;
            WORD rawSpritePattern = 0;

            // Computed values
            WORD name = 0;
            WORD color = 0;
            WORD pattern = 0;
            WORD spriteAttr = 0;
            WORD spritePattern = 0;

            // GRAPH1 & GRAPH2
            WORD patterns[3] = {};
            WORD colors[3] = {};

            void Reset();
            void Update(VideoMode mode);
        } m_tables;

        // Sprites
        Sprite* m_sprites = nullptr;
        const Sprite* GetSprite(int index) const { return m_sprites + index; }
        WORD GetSpritePatternBase(BYTE name) const { return m_tables.spritePattern + (8 * (name & Sprite::GetNameMask())); }
        void UpdateSpriteData();

        // Draw max 4 sprites per line
        std::array<const Sprite*, 4> m_spriteDrawList = {};

        using SpriteLine = std::array<uint32_t, H_TOTAL>;
        SpriteLine m_spritePixels;

        void UpdateSpriteDrawList();

        // Colors
        static const uint32_t s_palette[16];
        constexpr uint32_t GetColor(BYTE index) const { return s_palette[index]; }

        BYTE m_fgColor = 0;
        BYTE m_bgColor = 0;

        int m_currX = 0;
        int m_currY = 0;

        WORD m_currName = 0;

        void DrawGraph();
        void DrawSpriteLine();
        void DrawSpritePixel(uint32_t*& dest, bool bit, uint32_t color);
    };
}
