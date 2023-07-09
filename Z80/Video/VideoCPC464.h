#pragma once
#include <Video/Video.h>
#include "CRTControllerCPC464.h"

namespace video
{
    class VideoCPC464 : public Video, public crtc_6845::EventHandler
    {
    public:
        VideoCPC464();

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

        //bool IsCursor() const;

    private:
        CRTControllerCPC464 m_crtc;
    };
}
