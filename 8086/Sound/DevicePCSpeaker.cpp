#include "stdafx.h"

#include <Sound/Sound.h>
#include "DevicePCSpeaker.h"
#include "../Hardware/Device8254.h"
#include "../Hardware/Device8255.h"

using sound::SOUND;

namespace beeper
{
	DevicePCSpeaker::DevicePCSpeaker() : Logger("PCbeep")
	{
	}

	DevicePCSpeaker::~DevicePCSpeaker()
	{
	}

	void DevicePCSpeaker::Init(ppi::DevicePPI* ppi, pit::Device8254* pit)
	{
		assert(ppi);
		assert(pit);
		m_8254 = pit;
		m_ppi = ppi;
	}

	void DevicePCSpeaker::Tick(WORD mixWithL, WORD mixWithR)
	{
		static int sample = 0;
		static int32_t avg = 0;

		WORD speakerData = (m_ppi->IsSoundON() && m_8254->GetCounter(2).GetOutput()) ? 1024 : 0;
		SOUND().PlayStereo(speakerData + mixWithL, speakerData + mixWithR);
	}
}
