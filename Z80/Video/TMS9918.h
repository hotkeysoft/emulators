#pragma once
#include <Video/Video.h>
#include <Serializable.h>
#include <CPU/MemoryBlock.h>

namespace video::vdp
{
    enum class VideoMode { GRAPH_1, GRAPH_2, MULTICOLOR, TEXT };

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

        bool IsInterrupt() const { return m_config.interruptEnabled && m_interrupt; };

        // emul::Serializable
        virtual void Serialize(json& to);
        virtual void Deserialize(const json& from);

    protected:
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

        bool m_interrupt = false;

        // Table base addresses
        struct Tables
        {
            WORD name = 0;
            WORD color = 0;
            WORD patternGen = 0;
            WORD spriteAttr = 0;
            WORD spritePattern = 0;
        } m_baseAddr;

        static const uint32_t s_palette[16];
        constexpr uint32_t GetColor(BYTE index) const { return s_palette[index]; }

        uint32_t m_fgColor = GetColor(0);
        uint32_t m_bgColor = GetColor(0);

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

        int m_currX = 0;
        int m_currY = 0;

        const BYTE* m_currName = nullptr;
        const BYTE* m_currPattern = nullptr;
        const BYTE* m_currColor = nullptr;

        void DrawMode1();
    };
}
