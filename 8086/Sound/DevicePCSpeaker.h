#pragma once

#include "../Hardware/Device8254.h"
#include "../Hardware/Device8255.h"

namespace beeper
{	
	class DevicePCSpeaker : public Logger
	{
	public:
		DevicePCSpeaker();
		~DevicePCSpeaker();

		DevicePCSpeaker(const DevicePCSpeaker&) = delete;
		DevicePCSpeaker& operator=(const DevicePCSpeaker&) = delete;
		DevicePCSpeaker(DevicePCSpeaker&&) = delete;
		DevicePCSpeaker& operator=(DevicePCSpeaker&&) = delete;

		void Init(ppi::Device8255* ppi, pit::Device8254* pit);

		void Tick(WORD mixWith = 0);

	protected:
		ppi::Device8255* m_8255 = nullptr;
		pit::Device8254* m_8254 = nullptr;
	};
}
