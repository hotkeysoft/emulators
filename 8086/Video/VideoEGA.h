#pragma once

#include "../Common.h"
#include "../CPU/MemoryBlock.h"
#include "Video.h"
#include "CRTControllerEGA.h"
#include "GraphControllerEGA.h"
#include "AttributeControllerEGA.h"
#include "SequencerEGA.h"
#include "MemoryEGA.h"

#include <map>

using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

namespace video
{
	class VideoEGA : public Video, public crtc_ega::EventHandler
	{
	public:
		const char* GetDefaultVideoMode() const { return "ega"; }

		VideoEGA(const char* mode, memory_ega::RAMSIZE ramsize, WORD baseAddress, WORD baseAddressMono, WORD baseAddressColor);

		VideoEGA() = delete;
		VideoEGA(const VideoEGA&) = delete;
		VideoEGA& operator=(const VideoEGA&) = delete;
		VideoEGA(VideoEGA&&) = delete;
		VideoEGA& operator=(VideoEGA&&) = delete;

		virtual const std::string GetID() const override { return "ega"; }
		virtual const std::string GetDisplayName() const override { return "EGA"; }

		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;
		virtual void Tick() override;	

		virtual void EnableLog(SEVERITY minSev = LOG_INFO) override;

		// crtc::EventHandler
		virtual void OnChangeMode() override;
		virtual void OnRenderFrame() override;
		virtual void OnNewFrame() override;
		virtual void OnEndOfRow() override;

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(json& from) override;

		virtual uint32_t GetBackgroundColor() const override { return m_attrController.GetData().overscanColor; }
		virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;
		virtual bool IsEnabled() const override { return !m_misc.disableVideo; }

		virtual bool IsHSync() const override { return m_crtc.IsHSync(); }
		virtual bool IsVSync() const override { return m_crtc.IsVSync(); }
		virtual bool IsDisplayArea() const override { return m_crtc.IsDisplayArea(); }

	protected:
		memory_ega::RAMSIZE m_ramSize;
		WORD m_baseAddress;
		WORD m_baseAddressMono;
		WORD m_baseAddressColor;

		void InternalTick();

		bool IsCursor() const;

		void DisconnectRelocatablePorts(WORD base);
		void ConnectRelocatablePorts(WORD base);

		struct MISCRegister
		{
			bool color = false; // 1(color): maps port 0x3Dx, 0(mono): maps ports 0x3Bx
			bool enableRAM = false; // 1:enable RAM access from CPU
			enum class ClockSelect { CLK_14 = 0, CLK_16, CLK_EXT, CLK_UNUSED } clockSel = ClockSelect::CLK_14;
			bool disableVideo = false; // 1:disable output, 0:enable output
			bool pageHigh = false; // Select lo/hi page in odd/even modes
			bool hSyncPolarity = false;
			bool vSyncPolarity = false;
		} m_misc;
		void WriteMiscRegister(BYTE value);

		void WriteFeatureControlRegister(BYTE value);
		BYTE ReadStatusRegister0();
		BYTE ReadStatusRegister1();
	
		// Sequencer
		seq_ega::Sequencer m_sequencer;

		// Graphics Controllers
		graph_ega::GraphController m_graphController;

		// Attribute Controller
		attr_ega::AttrController m_attrController;

		emul::MemoryBlock m_egaROM;
		memory_ega::MemoryEGA m_egaRAM;

		ADDRESS GetBaseAddress() { return m_crtc.GetMemoryAddress(); }

		uint32_t GetIndexedColor(BYTE index) const { return m_attrController.GetData().palette[index]; }

		void DrawTextMode();
		void DrawGraphMode();
		void DrawGraphModeCGA4();

		crtc_ega::CRTController m_crtc;

		struct VideoMode
		{
			const char* description = "default";
			bool dipSwitches[4] = { false, true,  true,  false };
		} m_videoMode;

		typedef std::map<std::string, VideoMode> VideoModes;

		// Dual video cards not supported, only allow modes where EGA=primary
		const VideoModes m_videoModes = {
			{ "cga40",  { "CGA 40x25",     { true,  false, false, true  } } },
			{ "cga80",  { "CGA 80x25",     { false, false, false, true  } } },
			{ "egaemu", { "EGA Emulation", { true,  true,  true,  false } } },
			{ "ega",    { "EGA Hi-Res",    { false, true,  true,  false } } },
			{ "mda",    { "MDA",           { true,  false, true,  false } } },
		};
	};
}
