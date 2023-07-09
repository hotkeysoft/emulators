#pragma once
#include <Video/CRTController6845.h>

namespace video
{
    class CRTControllerCPC464 : public crtc_6845::CRTController
    {
    public:
        CRTControllerCPC464() : CRTController(0), Logger("CRTC") {}

        virtual void Init() override
        {
			Reset();

			// CRTC Register Select
			Connect("x0xxxx00", static_cast<PortConnector::OUTFunction>(&CRTControllerCPC464::SelectCRTCRegister));

			// CRTC Register Data
			Connect("x0xxxx01", static_cast<PortConnector::OUTFunction>(&CRTControllerCPC464::WriteCRTCData));
			Connect("x0xxxx11", static_cast<PortConnector::INFunction>(&CRTControllerCPC464::ReadCRTCData));
        }
    };
}
