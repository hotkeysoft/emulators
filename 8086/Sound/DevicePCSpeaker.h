#pragma once

namespace pit { class Device8254; }
namespace ppi { class DevicePPI; }

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

		void Init(ppi::DevicePPI* ppi, pit::Device8254* pit);

		void Tick(WORD mixWith = 0) { Tick(mixWith, mixWith); }
		void Tick(WORD mixWithL, WORD mixWithR);

	protected:
		ppi::DevicePPI* m_ppi = nullptr;
		pit::Device8254* m_8254 = nullptr;
	};
}
