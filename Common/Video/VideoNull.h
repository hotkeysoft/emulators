#pragma once
#include <Video/Video.h>

namespace video
{
    class VideoNull : public Video
    {
    public:
        VideoNull() : Video(), Logger("vidNULL") {};

        VideoNull(const VideoNull&) = delete;
        VideoNull& operator=(const VideoNull&) = delete;
        VideoNull(VideoNull&&) = delete;
        VideoNull& operator=(VideoNull&&) = delete;

        virtual const std::string GetID() const override { return "videoNULL"; };

        virtual void Tick() override { }

        virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override
        {
            Video::Init(memory, charROM, forceMono);
            InitFrameBuffer(width, height);
        }

        virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override { return { 0, 0, width, height }; }
        virtual bool IsEnabled() const override { return true; }

        virtual bool IsVSync() const override { return false; }
        virtual bool IsHSync() const override { return false; }
        virtual bool IsDisplayArea() const override { return true; }

    protected:
        WORD width = 320;
        WORD height = 200;
    };
}
