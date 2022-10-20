#pragma once
#include <Video/Video.h>
#include <Serializable.h>
#include <CPU/MemoryBlock.h>

namespace video
{
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

        bool IsInterrupt() { return false; };

        void Tick();

        // emul::Serializable
        virtual void Serialize(json& to);
        virtual void Deserialize(const json& from);

    protected:
        video::Video* m_video = nullptr;

        emul::MemoryBlock m_vram;

        // TODO: Adjust with vram size
        const WORD m_addressMask = 0x3FFF;

        bool m_dataFlipFlop = false;
        BYTE m_tempData = 0;

        WORD m_currReadAddress = 0;
        WORD m_currWriteAddress = 0;

    };
}
