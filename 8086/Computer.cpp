#include "Common.h"
#include "Computer.h"
#include "Config.h"
#include "IO/DeviceKeyboard.h"
#include "Storage/DeviceHardDrive.h"
#include "Video/VideoCGA.h"
#include "Video/VideoMDA.h"
#include "Video/VideoHGC.h"
#include "Video/VideoPCjr.h"
#include "Video/VideoTandy.h"

#include <assert.h>

using cfg::Config;

namespace emul
{
	Computer::Computer(Memory& memory, MemoryMap& mmap) :
		Logger("PC"),
		CPU8086(m_memory, m_map),
		m_memory(emul::CPU8086_ADDRESS_BITS),
		m_hddROM("hdd", 8192, MemoryType::ROM)
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

	void Computer::InitVideo(const std::string& defaultMode, const VideoModes& supported)
	{
		assert(m_video == nullptr);
		std::string mode = Config::Instance().GetValueStr("video", "mode");

		if (supported.empty())
		{
			mode = defaultMode;
		}
		else if (supported.find(mode) == supported.end())
		{
			LogPrintf(LOG_WARNING, "Invalid or unsupported video mode [%s], using default [%s]", mode.c_str(), defaultMode.c_str());
			mode = defaultMode;
		}

		if (mode == "cga")
		{
			m_video = new video::VideoCGA(0x3D0);
		}
		else if (mode == "mda")
		{
			m_video = new video::VideoMDA(0x3B0);
		}
		else if (mode == "hgc")
		{
			m_video = new video::VideoHGC(0x3B0);
		}
		else if (mode == "pcjr")
		{
			m_video = new video::VideoPCjr(0x3D0);
		}
		else if (mode == "tga")
		{
			m_video = new video::VideoTandy(0x3D0);
		}
		else
		{
			throw std::exception("Invalid video mode mode");
		}
		assert(m_video);
		m_video->EnableLog(Config::Instance().GetLogLevel("video"));

		LogPrintf(LOG_INFO, "Video mode: [%s]", mode.c_str());

		uint32_t border = Config::Instance().GetValueInt32("video", "border", 10);
		LogPrintf(LOG_INFO, "Border: [%d]", border);

		std::string charROM = Config::Instance().GetValueStr("video", "charrom", "data/XT/CGA_CHAR.BIN");
		// Check for override
		std::string overrideKey = "charrom." + mode;
		charROM = Config::Instance().GetValueStr("video", overrideKey.c_str(), charROM.c_str());

		LogPrintf(LOG_INFO, "Character ROM: [%s]", charROM.c_str());

		m_video->Init(&m_memory, charROM.c_str(), border);
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

	void Computer::InitJoystick(WORD baseAddress, size_t baseClock)
	{
		m_joystick = new joy::DeviceJoystick(baseAddress, baseClock);
		m_joystick->EnableLog(Config::Instance().GetLogLevel("joystick"));
		m_joystick->Init();
	}

	void Computer::InitHardDrive(hdd::DeviceHardDrive* hdd)
	{
		assert(hdd);
		m_hardDrive = hdd;

		m_hardDrive->EnableLog(Config::Instance().GetLogLevel("hdd"));
		m_hardDrive->Init();

		for (int i = 0; i < 2; i++)
		{
			const HardDriveImageInfo& img = GetHardDriveImageInfo(i + 1);
			if (img.set)
			{
				m_hardDrive->LoadDiskImage(i, img.type, img.file.c_str());
			}
		}

		std::string romFile = Config::Instance().GetValueStr("hdd", "rom");
		if (romFile.empty())
		{
			romFile = "data/hdd/WD1002S-WX2_62-000042-11.bin";
		}
		m_hddROM.LoadFromFile(romFile.c_str());
		m_memory.Allocate(&m_hddROM, 0xC8000);
	}

	Computer::HardDriveImageInfo Computer::GetHardDriveImageInfo(int id)
	{
		char key[16];
		sprintf(key, "hdd.%d", id);

		std::string imageStr = Config::Instance().GetValueStr("hdd", key);
		HardDriveImageInfo info;
		if (imageStr.size())
		{
			std::stringstream ss(imageStr);
			std::string type;
			std::getline(ss, type, ',');
			if (type.size())
			{
				info.type = atoi(type.c_str());
			}
			else
			{
				LogPrintf(LOG_ERROR, "GetHDDImageConfig: Error parsing hdd image type for key [hdd][%s]", key);
			}
			std::getline(ss, info.file);
			if (info.file.size())
			{
				info.set = true;
			}
			else
			{
				LogPrintf(LOG_ERROR, "GetHDDImageConfig: Error parsing hdd image file for key [hdd][%s]", key);
			}
		}

		return info;
	}
}
