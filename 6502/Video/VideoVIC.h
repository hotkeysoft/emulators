#pragma once
#include <Video/Video.h>
#include <CPU/IOConnector.h>

namespace via
{
    class Device6522PET;
}

namespace video
{
    enum class VICRegister
    {
        ORIGIN_X = 0,
        ORIGIN_Y,
        COLUMNS,
        ROWS,
        RASTER,
        BASE_ADDRESS,
        LIGHTPEN_X,
        LIGHTPEN_Y,
        POT_X,
        POT_Y,
        AUDIO_FREQ1,
        AUDIO_FREQ2,
        AUDIO_FREQ3,
        AUDIO_FREQ4,
        AUDIO_AMPLITUDE,
        COLOR_CONTROL,

        _REGISTER_COUNT
    };

    class VideoVIC : public Video, public emul::IOConnector
    {
    public:
        VideoVIC();

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false);
        virtual void Reset() override;

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

        // emul::Serializable
        virtual void Serialize(json& to) override;
        virtual void Deserialize(const json& from) override;

    protected:
        // CR0
        BYTE ReadScreenOriginX();
        void WriteScreenOriginX(BYTE value);

        // CR1
        BYTE ReadScreenOriginY();
        void WriteScreenOriginY(BYTE value);

        // CR2
        BYTE ReadColumns();
        void WriteColumns(BYTE value);

        // CR3
        BYTE ReadRows();
        void WriteRows(BYTE value);

        // CR4
        BYTE ReadRaster();
        void WriteRaster(BYTE value);

        // CR5
        BYTE ReadBaseAddress();
        void WriteBaseAddress(BYTE value);

        // CR6
        BYTE ReadLightPenX();
        void WriteLightPenX(BYTE value);

        // CR7
        BYTE ReadLightPenY();
        void WriteLightPenY(BYTE value);

        // CR8
        BYTE ReadPotX();
        void WritePotX(BYTE value);

        // CR9
        BYTE ReadPotY();
        void WritePotY(BYTE value);

        // CRA
        BYTE ReadAudioFreq1();
        void WriteAudioFreq1(BYTE value);

        // CRB
        BYTE ReadAudioFreq2();
        void WriteAudioFreq2(BYTE value);

        // CRC
        BYTE ReadAudioFreq3();
        void WriteAudioFreq3(BYTE value);

        // CRD
        BYTE ReadAudioFreq4();
        void WriteAudioFreq4(BYTE value);

        // CRE
        BYTE ReadAudioAmplitude();
        void WriteAudioAmplitude(BYTE value);

        // CRF
        BYTE ReadColorControl();
        void WriteColorControl(BYTE value);

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

        // RAW vic registers
        BYTE ReadVICRegister(VICRegister reg) const { return m_rawVICRegisters[(int)reg]; }
        void WriteVICRegister(VICRegister reg, BYTE value) { m_rawVICRegisters[(int)reg] = value; }
        std::array<BYTE, (int)VICRegister::_REGISTER_COUNT> m_rawVICRegisters;

        void UpdateBaseAddress();
        void AdjustA13(ADDRESS& addr) const;

        // Computed by UpdateBaseAddress()
        ADDRESS m_matrixBaseAddress = 0;
        ADDRESS m_colorBaseAddress = 0;
        ADDRESS m_charBaseAddress = 0;

        ADDRESS m_currChar = 0;

        void DrawChar();

        uint32_t m_bgColor = 0xFF000000;
        uint32_t m_fgColor = 0xFFFFFFFF;

        uint32_t m_currX = 0;
        uint32_t m_currY = 0;
        BYTE m_currRow = 0;
    };
}
