#pragma once

#include "../Common.h"
#include "../CPU/MemoryBlock.h"
#include "Video.h"
#include "CRTControllerVGA.h"
#include "GraphControllerVGA.h"
#include "AttributeControllerVGA.h"
#include "SequencerVGA.h"
#include "MemoryVGA.h"

#include <map>

using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

namespace video
{
	class VideoVGA : public Video, public crtc_vga::EventHandler
	{
	public:
		VideoVGA(WORD baseAddress, WORD baseAddressMono, WORD baseAddressColor);

		VideoVGA() = delete;
		VideoVGA(const VideoVGA&) = delete;
		VideoVGA& operator=(const VideoVGA&) = delete;
		VideoVGA(VideoVGA&&) = delete;
		VideoVGA& operator=(VideoVGA&&) = delete;

		virtual const std::string GetID() const override { return "vga"; }
		virtual const std::string GetDisplayName() const override { return "VGA"; }

		virtual void Reset() override;
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
		virtual void Deserialize(const json& from) override;

		virtual uint32_t GetBackgroundColor() const override { return m_attrController.GetData().overscanColor; }
		virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;
		virtual bool IsEnabled() const override { return !m_misc.disableVideo; }

		virtual bool IsHSync() const override { return m_crtc.IsHSync(); }
		virtual bool IsVSync() const override { return m_crtc.IsVSync(); }
		virtual bool IsDisplayArea() const override { return m_crtc.IsDisplayArea(); }

	protected:
		memory_vga::RAMSIZE m_ramSize;
		WORD m_baseAddress;
		WORD m_baseAddressMono;
		WORD m_baseAddressColor;

		bool m_syncPelPanning = false;
		BYTE m_newPelPanning = 0;

		void InternalTick();

		bool IsCursor() const;

		void DisconnectPorts();
		void ConnectPorts();

		void DisconnectRelocatablePorts(WORD base);
		void ConnectRelocatablePorts(WORD base);

		void WriteNull(BYTE) {}

		struct SetupControlRegister // port 0x46E8
		{
			bool enable = false; // VGA Enable
			bool setupMode = false; // VGA Setup Mode
		} m_setup;
		void WriteSetupRegister(BYTE value);

		// Global Enable Register (port 0x102)
		bool m_globalEnable = false; // 1: vga awake, 0: vga sleep
		void WriteGlobalEnableRegister(BYTE value);

		struct MISCRegister
		{
			bool color = false; // 1(color): maps port 0x3Dx, 0(mono): maps ports 0x3Bx
			bool enableRAM = false; // 1:enable RAM access from CPU
			enum class ClockSelect { CLK_25 = 0, CLK_28, CLK_EXT, CLK_UNUSED } clockSel = ClockSelect::CLK_25;

			// TODO: Undefined in IBM doc
			bool disableVideo = false; // 1:disable output, 0:enable output
			bool pageHigh = false; // Select lo/hi page in odd/even modes

			bool hSyncPolarity = false;
			bool vSyncPolarity = false;
		} m_misc;
		BYTE ReadMiscRegister();
		void WriteMiscRegister(BYTE value);

		void WriteFeatureControlRegister(BYTE value);
		BYTE ReadStatusRegister0();
		BYTE ReadStatusRegister1();
	
		// Sequencer
		seq_vga::Sequencer m_sequencer;

		// Graphics Controllers
		graph_vga::GraphController m_graphController;

		// Attribute Controller
		attr_vga::AttrController m_attrController;

		emul::MemoryBlock m_vgaROM;
		memory_vga::MemoryVGA m_vgaRAM;

		ADDRESS GetBaseAddress() { return m_crtc.GetMemoryAddress(); }

		uint32_t GetIndexedColor(BYTE index) const { return m_attrController.GetData().palette[index]; }

		void DrawTextMode();
		void DrawTextModeMDA();
		void DrawGraphMode();
		void DrawGraphModeCGA4();

		crtc_vga::CRTController m_crtc;
	};
}
