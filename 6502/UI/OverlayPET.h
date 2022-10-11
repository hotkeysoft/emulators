#pragma once
#include <UI/Overlay.h>

namespace ui
{
    class OverlayPET : public Overlay
    {
    public:
        virtual void SetPC(emul::ComputerBase* pc) override;
        virtual void OnClick(CoreUI::WidgetRef widget) override;

    protected:
        void LoadPRG();
    };
}
