#include "DeviceKeyboard.h"
#include <assert.h>

namespace kbd
{
	DeviceKeyboard::DeviceKeyboard() : Logger("KBD")
	{
	}

	void DeviceKeyboard::Init(ppi::Device8255* ppi, pic::Device8259* pic)
	{
		assert(ppi);
		assert(pic);
		m_ppi = ppi;
		m_pic = pic;
	}
}
