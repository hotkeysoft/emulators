#pragma once
#include <Video/Video.h>
#include "CRTControllerCPC.h"

namespace video::cpc
{
    class EventHandler
    {
    public:
        virtual void OnLowROMChange(bool load) {}
        virtual void OnHighROMChange(bool load) {}
    };

    class VideoCPC : public Video, public crtc_6845::EventHandler
    {
    public:
        VideoCPC(emul::MemoryBlock* ram);

        VideoCPC(const VideoCPC&) = delete;
        VideoCPC& operator=(const VideoCPC&) = delete;
        VideoCPC(VideoCPC&&) = delete;
        VideoCPC& operator=(VideoCPC&&) = delete;

        virtual const std::string GetID() const override { return "vidCPC"; }

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false);

        void SetEventHandler(video::cpc::EventHandler* handler) { m_events = handler; }

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

        bool IsInterrupt() const { return m_isInterrupt;  }
        void InterruptAcknowledge();

    protected:
        void Write(BYTE value);

        ADDRESS GetBaseAddress() { return m_baseAddress + (((m_crtc.GetData().rowAddress * 0x800) + (m_crtc.GetMemoryAddress10() * 2u)) & 0xFFFF); }

        uint32_t GetColor(BYTE index) const { return m_palette[m_pens[index]]; }
        virtual uint32_t GetBackgroundColor() const override { return GetColor(PEN_BORDER); }

        void Draw160x200x16(); // Mode 0
        void Draw320x200x4();  // Mode 1
        void Draw640x200x2(); // Mode 2
        void Draw160x200x4(); // Mode 3 (unofficial)

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
        ADDRESS m_baseAddress = 0;

        // [0..15]=pens, [16]=border
        static const int PEN_COUNT = 16;
        static const int PEN_BORDER = PEN_COUNT;
        std::array<int, PEN_COUNT + 1> m_pens = { 0 };
        int m_currPen = 0;

        const uint32_t m_palette[32] = {
            0xFF6E7D6B, 0xFF6E7B6D, 0xFF00F36B, 0xFFF3F36D,
            0xFF00026B, 0xFFF00268, 0xFF007868, 0xFFF37D6B,
            0xFFF30268, 0xFFF3F36B, 0xFFF3F30D, 0xFFFFF3F9,
            0xFFF30506, 0xFFF302F4, 0xFFF37D0D, 0xFFFA80F9,
            0xFF000268, 0xFF02F36B, 0xFF02F001, 0xFF0FF3F2,
            0xFF000201, 0xFF0C02F4, 0xFF027801, 0xFF0C7BF4,
            0xFF690268, 0xFF71F36B, 0xFF71F504, 0xFF71F3F4,
            0xFF6C0201, 0xFF6C02F2, 0xFF6E7B01, 0xFF6E7BF6 };

        bool m_romHighEnabled = false;
        bool m_romLowEnabled = true;

        BYTE m_interruptCounter = 0;
        bool m_isInterrupt = false;

    private:
        video::cpc::EventHandler* m_events = nullptr;

        CRTControllerCPC m_crtc;
    };
}
