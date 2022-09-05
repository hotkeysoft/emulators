#pragma once
#include "Video.h"

namespace video
{
    class VideoZXSpectrum : public Video
    {
    public:
        VideoZXSpectrum();

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false);

        virtual const std::string GetID() const override { return "zxspectrum"; }
        virtual void Tick() override;

        virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;

        virtual bool IsEnabled() const override { return true; }

        virtual bool IsVSync() const override { return m_vSync; }
        virtual bool IsHSync() const override { return m_hSync; }
        virtual bool IsDisplayArea() const override { return !IsVSync() && !IsHSync(); }

        void VSync();

    protected:
        const ADDRESS PIXELS_BASE = 0x4000;
        const ADDRESS ATTR_BASE = 0x5800;
        ADDRESS m_pixelAddress = PIXELS_BASE;
        ADDRESS m_attrAddress = ATTR_BASE;
        void UpdateBaseAddress()
        {
            m_pixelAddress = PIXELS_BASE;
            const uint32_t y = m_currY - 32; // TODO, hardcoded
            m_pixelAddress |= (y & 0b11000000) << 5; // Y7Y6
            m_pixelAddress |= (y & 0b00111000) << 2; // Y5Y4Y3
            m_pixelAddress |= (y & 0b00000111) << 8; // Y2Y1Y0

            m_attrAddress = ATTR_BASE;
            m_attrAddress += (y / 8) * 32;
        }

        void DrawChar();

        bool m_vSync = false;
        bool m_hSync = false;

        uint32_t m_currX = 0;
        uint32_t m_currY = 0;

        uint32_t m_currFG = 0;
        uint32_t m_currBG = 0;

        const uint32_t m_palette0[8] = {
            0xFF000000, 0xFF0000D8, 0xFFD80000, 0xFFD800D8,
            0xFF00D800, 0xFF00D8D8, 0xFFD8D800, 0xFFD8D8D8
        };
        const uint32_t m_palette1[8] = {
            0xFF000000, 0xFF0000FF, 0xFFFF0000, 0xFFFF00FF,
            0xFF00FF00, 0xFF00FFFF, 0xFFFFFF00, 0xFFFFFFFF
        };

        uint32_t m_pixelClock = 0; // Temp
    };
}
