#pragma once
#include <Video/CRTController6845.h>

namespace video
{
    class CRTControllerCPC : public crtc_6845::CRTController
    {
    public:
        CRTControllerCPC() : CRTController(0), Logger("CRTC") {}

        virtual void Init() override
        {
			Reset();

			// CRTC Register Select
			Connect("x0xxxx00", static_cast<PortConnector::OUTFunction>(&CRTControllerCPC::SelectCRTCRegister), true);

			// CRTC Register Data
			Connect("x0xxxx01", static_cast<PortConnector::OUTFunction>(&CRTControllerCPC::WriteCRTCData), true);
			Connect("x0xxxx11", static_cast<PortConnector::INFunction>(&CRTControllerCPC::ReadCRTCData), true);
        }
    };
}
