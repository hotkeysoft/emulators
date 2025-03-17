#pragma once
#include <CPU/MemoryBlockBase.h>
#include <Video/VideoMC10.h>
#include <IO/DeviceKeyboardMC10.h>
#include <cassert>

namespace io::mc10
{
	class IOBlockMC10: public emul::MemoryBlockBase
	{
	public:
		IOBlockMC10() : MemoryBlockBase("IO_MC10", 0x4000, emul::MemoryType::IO) {}
		virtual ~IOBlockMC10() {};

		void Init(bool* soundData, video::Video* video, kbd::DeviceKeyboardMC10* keyboard)
		{
			assert(soundData);
			m_soundData = soundData;

			m_video = dynamic_cast<video::VideoMC10*>(video);
			assert(m_video);

			assert(keyboard);
			m_keyboard = keyboard;
		}

		int GetKeyboardScanRow() const { return m_keyboardScanRow; }
		void SetKeyboardScanRow(int row) { m_keyboardScanRow = row; }

	protected:
		virtual BYTE read(emul::ADDRESS offset) const override
		{
			LogPrintf(LOG_TRACE, "Read Keyboard");
			const BYTE data = m_keyboard->GetRowData(m_keyboardScanRow);

			return ~data;
		}

		virtual void write(emul::ADDRESS offset, BYTE data) override
		{
			assert(m_soundData);
			LogPrintf(LOG_DEBUG, "Write VDP/Sound");
			*m_soundData = emul::GetBit(data, 7);

			m_video->SetCSS(emul::GetBit(data, 6));
			m_video->SetAlphaGraph(emul::GetBit(data, 5));
			m_video->SetGM0(emul::GetBit(data, 4));
			m_video->SetGM1(emul::GetBit(data, 3));
			m_video->SetGM2(emul::GetBit(data, 2));
			m_video->SetIntExt(emul::GetBit(data, 2));
		}

		bool* m_soundData = nullptr;
		video::VideoMC10* m_video = nullptr;
		kbd::DeviceKeyboardMC10* m_keyboard = nullptr;
		int m_keyboardScanRow = 0;
    };
}