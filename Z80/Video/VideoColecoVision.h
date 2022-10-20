#pragma once
#include <Video/Video.h>
#include "TMS9918.h"

namespace video
{
    class VideoColecoVision : public Video
    {
    public:
        VideoColecoVision();

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false);

        virtual const std::string GetID() const override { return "colecovision"; }
        virtual void Tick() override
        {
            // VDP clock is 3x cpu clk
            m_vdp.Tick();
            m_vdp.Tick();
            m_vdp.Tick();
        }

        virtual void EnableLog(SEVERITY severity) override;

        virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;

        virtual bool IsEnabled() const override { return true; }

        virtual bool IsVSync() const override { return false; }
        virtual bool IsHSync() const override { return false; }
        virtual bool IsDisplayArea() const override { return !IsVSync() && !IsHSync(); }

        // emul::Serializable
        virtual void Serialize(json& to) override;
        virtual void Deserialize(const json& from) override;

    protected:
        BYTE Read0() { return m_vdp.ReadVRAMData(); }
        BYTE Read1() { return m_vdp.ReadStatus(); }

        void Write0(BYTE value) { m_vdp.WriteVRAMData(value); }
        void Write1(BYTE value) { m_vdp.Write(value); }

        TMS9918 m_vdp;
    };
}
