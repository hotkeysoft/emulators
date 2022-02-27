#pragma once

#include "../Common.h"
#include "../CPU/MemoryBlock.h"
#include "Video.h"
#include "CRTControllerEGA.h"
#include "MemoryEGA.h"

using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

namespace video
{
	class VideoEGA : public Video, public crtc_ega::EventHandler
	{
	public:
		VideoEGA(memory_ega::RAMSIZE ramsize, WORD baseAddress, WORD baseAddressMono, WORD baseAddressColor);

		VideoEGA() = delete;
		VideoEGA(const VideoEGA&) = delete;
		VideoEGA& operator=(const VideoEGA&) = delete;
		VideoEGA(VideoEGA&&) = delete;
		VideoEGA& operator=(VideoEGA&&) = delete;

		virtual const std::string GetID() const override { return "ega"; }

		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;
		virtual void Tick() override;

		virtual void EnableLog(SEVERITY minSev = LOG_INFO) override;

		// crtc::EventHandler
		virtual void OnRenderFrame() override;
		virtual void OnNewFrame() override;
		virtual void OnEndOfRow() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(json& from) override;

		virtual uint32_t GetBackgroundColor() const override { return GetMonitorPalette()[0]; }
		virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;
		virtual bool IsEnabled() const override { return true; }

		virtual bool IsHSync() const override { return m_crtc.IsHSync(); }
		virtual bool IsVSync() const override { return m_crtc.IsVSync(); }
		virtual bool IsDisplayArea() const override { return m_crtc.IsDisplayArea(); }

	protected:
		memory_ega::RAMSIZE m_ramSize;
		WORD m_baseAddress;
		WORD m_baseAddressMono;
		WORD m_baseAddressColor;

		bool IsCursor() const;

		void DisconnectRelocatablePorts(WORD base);
		void ConnectRelocatablePorts(WORD base);

		struct MISCRegister
		{
			bool color = false;
			bool enableRAM = false;
			enum class ClockSelect { CLK_14 = 0, CLK_16, CLK_EXT, CLK_UNUSED } clockSel = ClockSelect::CLK_14;
			bool disableVideo = false;
			bool pageHigh = false; // Select lo/hi page in odd/even modes
			bool hSyncPolarity = false; // Unused
			bool vSyncPolarity = false; // Unused
		} m_misc;
		void WriteMiscRegister(BYTE value);

		void WriteFeatureControlRegister(BYTE value);
		BYTE ReadFeatureControlRegister() { return 0xFF; } // Register is write only, just to shut the noise
		BYTE ReadStatusRegister0();
		BYTE ReadStatusRegister1();

		// Sequencer
		enum class SequencerAddress {
			SEQ_RESET = 0,
			SEQ_CLOCKING_MODE,
			SEQ_MAP_MASK,
			SEQ_CHARMAP_SELECT,
			SEQ_MEMORY_MODE,

			SEQ_INVALID
		} m_seqAddress = SequencerAddress::SEQ_INVALID;
		void WriteSequencerAddress(BYTE value);
		
		// Dispatches value to functions below
		void WriteSequencerValue(BYTE value);
		void WriteSequencerReset(BYTE value);

		struct ClockingMode
		{
			BYTE charWidth = 8; // 8 or 9
			bool lowBandwidth = false; // low: 2/5 mem cycles, hi: 4/5
			bool load16 = false; // 0: serializer load every clock, 1: every 2 clock
			bool halfDotClock = false; // 0: normal dot clock, 1: half (for 320x200 modes)
		} m_clockingMode;
		void WriteSequencerClockingMode(BYTE value);

		BYTE m_mapMask = 0;
		void WriteSequencerMapMask(BYTE value);

		BYTE m_charMapSelectA = 0;
		BYTE m_charMapSelectB = 0;
		void WriteSequencerCharMapSelect(BYTE value);

		struct MemoryMode
		{
			bool alpha = false;
			bool extMemory = false; // 1: allows access to >64k ram
			bool sequential = false; // 0: Odd/Even access, 1: sequential
		} m_memoryMode;
		void WriteSequencerMemoryMode(BYTE value);

		enum class GraphControllerAddress
		{
			GRAPH_SET_RESET = 0,
			GRAPH_ENABLE_SET_RESET = 1,
			GRAPH_COLOR_COMPARE = 2,
			GRAPH_DATA_ROTATE = 3,
			GRAPH_READ_MAP_SELECT = 4,
			GRAPH_MODE = 5,
			GRAPH_MISC = 6,
			GRAPH_COLOR_DONT_CARE = 7,
			GRAPH_BIT_MASK = 8,

			_GRAPH_MAX_REG = GRAPH_BIT_MASK,
			GRAPH_INVALID_REG = 0xFF
		};


		enum class MemoryMap { A000_128K = 0, A000_64K, B000_32K, B800_32K };

		enum class RotateFunction { NONE = 0, AND, OR, XOR };

		struct GraphControllerRegisters
		{
			GraphControllerAddress currRegister = GraphControllerAddress::GRAPH_INVALID_REG;

			BYTE setReset = 0;
			BYTE enableSetReset = 0;
			
			BYTE colorCompare = 0;
			BYTE colorDontCare = 0;

			BYTE rotateCount = 0;
			RotateFunction rotateFunction = RotateFunction::NONE;

			BYTE mapSelect;

			BYTE writeMode = 0;
			bool readModeCompare = false;
			bool oddEven = false;
			bool shiftRegister = false;

			bool graphics = false;
			bool chainOddEven = false;
			MemoryMap memoryMap = MemoryMap::A000_128K;

			BYTE bitMask = 0;

		} m_graphController;
		void WriteGraphics1Position(BYTE value);
		void WriteGraphics2Position(BYTE value);
		void WriteGraphicsAddress(BYTE value);
		void WriteGraphicsValue(BYTE value);

		void MapMemory();

		emul::MemoryBlock m_egaROM;
		memory_ega::MemoryEGA m_egaRAM;

		ADDRESS GetBaseAddress() { return 0; }

		uint32_t GetIndexedColor(BYTE index) const { return GetMonitorPalette()[index]; }

		void DrawTextMode();

		crtc_ega::CRTController m_crtc;

		// TODO
		// When reading settings from table in manual, use on = true, off = false
		// 
		bool m_dipSwitches[4] = { true, false, false, true }; // EGA = Primary, 40x25; MDA = Secondary
		//bool m_dipSwitches[4] = { true, false, true, false }; // EGA = Mono; CGA = Secondary 40x25
	};
}