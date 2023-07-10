#pragma once
#include <Video/Video.h>
#include "CRTControllerCPC464.h"

namespace video
{
    class VideoCPC464 : public Video, public crtc_6845::EventHandler
    {
    public:
        VideoCPC464(emul::MemoryBlock* ram);

        VideoCPC464(const VideoCPC464&) = delete;
        VideoCPC464& operator=(const VideoCPC464&) = delete;
        VideoCPC464(VideoCPC464&&) = delete;
        VideoCPC464& operator=(VideoCPC464&&) = delete;

        virtual const std::string GetID() const override { return "cpc464"; }

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false);

        virtual void Reset() override;
        virtual void Tick() override;

        virtual void EnableLog(SEVERITY minSev = LOG_INFO) override;

        // crtc::EventHandler
        virtual void OnRenderFrame() override;
        virtual void OnNewFrame() override;
        virtual void OnEndOfRow() override;

        virtual void Serialize(json& to) override;
        virtual void Deserialize(const json& from) override;

        virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;

        virtual bool IsEnabled() const { return true; }

        virtual bool IsVSync() const { return m_crtc.IsVSync(); }
        virtual bool IsHSync() const { return m_crtc.IsHSync(); }
        virtual bool IsDisplayArea() const { return m_crtc.IsDisplayArea(); }

    protected:
        void Write(BYTE value);

        ADDRESS GetBaseAddress() { return m_baseAddress + (((m_crtc.GetData().rowAddress * 0x800) + (m_crtc.GetMemoryAddress10() * 2u)) & 0xFFFF); }

        uint32_t GetColor(BYTE index) const { return m_palette[m_pens[index]]; }
        virtual uint32_t GetBackgroundColor() const override { return GetColor(16); }

        void Draw160x200x16() {};
        void Draw320x200x4();
        void Draw640x200x2() {};
        void Draw160x200x4() {};

        const std::array<const char*, 4> m_modes = {
            "160x200x16",
            "320x200x4",
            "640x200x2",
            "160x200x4" };
        int m_mode = 0;
        void UpdateMode();

        //bool IsCursor() const;

        emul::MemoryBlock* m_ram = nullptr;
        void UpdateBaseAddress();
        ADDRESS m_baseAddress = 0xC000;

        // [0..15]=pens, [16]=border
        int m_currPen = 0;
        std::array<int, 17> m_pens = { 0 };

        const uint32_t m_palette[32] = {
            0xFF6E7D6B, 0xFF6E7B6D, 0xFF00F36B, 0xFFF3F36D,
            0xFF00026B, 0xFFF00268, 0xFF007868, 0xFFF37D6B,
            0xFFF30268, 0xFFF3F36B, 0xFFF3F30D, 0xFFFFF3F9,
            0xFFF30506, 0xFFF302F4, 0xFFF37D0D, 0xFFFA80F9,
            0xFF000268, 0xFF02F36B, 0xFF02F001, 0xFF0FF3F2,
            0xFF000201, 0xFF0C02F4, 0xFF027801, 0xFF0C7BF4,
            0xFF690268, 0xFF71F36B, 0xFF71F504, 0xFF71F3F4,
            0xFF6C0201, 0xFF6C02F2, 0xFF6E7B01, 0xFF6E7BF6 };

    private:
        CRTControllerCPC464 m_crtc;
    };
}
