#include "stdafx.h"

#include "DevicePCSpeaker.h"
#include "Sound.h"

using sound::SOUND;

namespace beeper
{
	DevicePCSpeaker::DevicePCSpeaker() : Logger("PCbeep")
	{
	}

	DevicePCSpeaker::~DevicePCSpeaker()
	{
	}

	void DevicePCSpeaker::Init(ppi::Device8255* ppi, pit::Device8254* pit)
	{
		assert(ppi);
		assert(pit);
		m_8254 = pit;
		m_8255 = ppi;
	}

	void DevicePCSpeaker::Tick(WORD mixWith)
	{
		static int sample = 0;
		static int32_t avg = 0;

		BYTE speakerData = (m_8255->IsSoundON() && m_8254->GetCounter(2).GetOutput()) ? 128 : 0;
		SOUND().Play(speakerData + mixWith);
	}
}
