#pragma once

#include "../Common.h"
#include "../CPU/MemoryBlock.h"
#include "../CPU/Memory.h"
#include "VideoMDA.h"

using emul::WORD;
using emul::BYTE;

namespace video
{
	// Hercules Graphics Card (HGC)
	//
	// Superset of the Monochrome Display Adapter (MDA)
	//
	// Adds 720x348 graphics mode
	// and some changes to the registers
	class VideoHGC : public VideoMDA
	{
	public:
		VideoHGC(WORD baseAddress);

		VideoHGC() = delete;
		VideoHGC(const VideoHGC&) = delete;
		VideoHGC& operator=(const VideoHGC&) = delete;
		VideoHGC(VideoHGC&&) = delete;
		VideoHGC& operator=(VideoHGC&&) = delete;

		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;

		virtual void OnChangeMode() override;
		virtual void OnNewFrame() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(json& from) override;

	protected:
		// Mode Control Register (Hercules)
		struct MODEControl
		{
			bool graphics = false;
			bool displayPage = false;
		} m_modeHGC;
		virtual void WriteModeControlRegister(BYTE value) override;

		// Status Register
		virtual BYTE ReadStatusRegister() override;

		void Draw720x348();

		// Graph mode banks
		BYTE* m_banks[4] = { nullptr, nullptr, nullptr, nullptr };
	};
}
