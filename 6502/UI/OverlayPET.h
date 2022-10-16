#pragma once
#include <UI/Overlay.h>
#include "PRGLoader.h"

namespace ui
{
    class OverlayPET : public Overlay
    {
    public:
        virtual void SetPC(emul::ComputerBase* pc) override;
        virtual void OnClick(CoreUI::WidgetRef widget) override;

    protected:
        emul::PRGLoader* GetPRGLoader() { return dynamic_cast<emul::PRGLoader*>(m_pc); };

        void LoadPRG();
        void UnloadPRG();
    };
}
