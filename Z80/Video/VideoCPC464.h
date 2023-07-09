#pragma once
#include <Video/Video.h>

namespace video
{
    class VideoCPC464 : public Video
    {
    public:
        VideoCPC464();

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false);

        virtual const std::string GetID() const override { return "colecovision"; }
        virtual void Tick() override
        {
            //m_vdp.Tick();

            //// VDP clock is 1.5x cpu clk
            //static bool half = false;
            //if (half) m_vdp.Tick();
            //half = !half;
        }

        virtual void EnableLog(SEVERITY severity) override;

        virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;

        virtual bool IsEnabled() const override { return true; }

        virtual bool IsVSync() const override { return false; }
        virtual bool IsHSync() const override { return false; }
        virtual bool IsDisplayArea() const override { return false; }

        bool IsInterrupt() const { return false; }

        // emul::Serializable
        virtual void Serialize(json& to) override;
        virtual void Deserialize(const json& from) override;

    protected:
        void Write(BYTE value);
    };
}
