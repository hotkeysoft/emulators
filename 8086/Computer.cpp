#include "Common.h"
#include "Computer.h"
#include "DeviceKeyboard.h"
#include "Config.h"

#include <assert.h>

using cfg::Config;

namespace emul
{
	Computer::Computer(Memory& memory, MemoryMap& mmap) :
		Logger("PC"),
		CPU8086(m_memory, m_map),
		m_memory(emul::CPU8086_ADDRESS_BITS)
	{
	}

	Computer::~Computer()
	{
		delete m_pit;
		delete m_pic;
		delete m_ppi;
	}

	void Computer::Reboot(bool hard)
	{
		if (hard)
		{
			Reset();
		}
		else
		{
			GetKeyboard().InputKey(0x1D); // CTRL
			GetKeyboard().InputKey(0x38); // ALT
			GetKeyboard().InputKey(0x53); // DELETE
		}
	}

	void Computer::InitSound()
	{
		m_pcSpeaker.EnableLog(Config::Instance().GetLogLevel("sound"));
		m_pcSpeaker.Init(m_ppi, m_pit);
		m_pcSpeaker.SetMute(Config::Instance().GetValueBool("sound", "mute"));
		std::string audioStream = Config::Instance().GetValueStr("sound", "raw");
		if (audioStream.size())
		{
			m_pcSpeaker.StreamToFile(true, audioStream.c_str());
		}
	}

	void Computer::InitPIT(pit::Device8254* pit)
	{
		assert(pit);
		m_pit = pit;
		m_pit->EnableLog(Config::Instance().GetLogLevel("pit"));
		m_pit->Init();
	}
	void Computer::InitPIC(pic::Device8259* pic)
	{
		assert(pic);
		m_pic = pic;
		m_pic->EnableLog(Config::Instance().GetLogLevel("pic"));
		m_pic->Init();
	}
	void Computer::InitPPI(ppi::Device8255* ppi)
	{
		assert(ppi);
		m_ppi = ppi;
		m_ppi->EnableLog(Config::Instance().GetLogLevel("ppi"));
		m_ppi->Init();
	}

}
