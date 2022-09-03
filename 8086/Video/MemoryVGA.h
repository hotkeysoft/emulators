#pragma once
#include <CPU/MemoryBlock.h>
#include <Serializable.h>

#include <array>

namespace graph_vga
{
	struct GraphControllerData;
}

namespace seq_vga
{
	struct SequencerData;
}

namespace memory_vga
{
	enum class RAMSIZE { VGA_256K = 262144 };

	class MemoryVGA : public emul::MemoryBlock, public emul::Serializable
	{
	public:
		MemoryVGA();
		virtual ~MemoryVGA() {}

		virtual void Clear(BYTE filler = 0) override;

		void SetGraphController(const graph_vga::GraphControllerData* graph) { m_graphData = graph; }
		void SetSequencer(const seq_vga::SequencerData* seq) { m_seqData = seq; }

		void Enable(bool enable) { m_enable = enable; }

		virtual BYTE read(emul::ADDRESS offset) override;
		virtual void write(emul::ADDRESS offset, BYTE data) override;

		// Direct access to ram for video card drawing
		BYTE readRaw(BYTE plane, emul::ADDRESS offset) { return m_planes[plane].read(offset); }

		// Character Maps
		void SelectCharMaps(BYTE selectA, BYTE selectB);

		const BYTE* GetCharMapB() const { return m_charMapA; }
		const BYTE* GetCharMapA() const { return m_charMapB; }

		virtual bool LoadFromFile(const char* file, WORD offset = 0) override;
		virtual bool Dump(emul::ADDRESS offset, emul::DWORD len, const char* outFile) const override;

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		RAMSIZE m_ramSize;
		bool m_enable = false;

		const graph_vga::GraphControllerData* m_graphData = nullptr;
		const seq_vga::SequencerData* m_seqData = nullptr;

		const emul::DWORD m_planeSize;
		const emul::ADDRESS m_planeAddressMask;

		MemoryBlock m_planes[4];
		std::array<BYTE, 4> m_dataLatches = { 0, 0, 0, 0 };

		const BYTE* m_charMapA = nullptr;
		const BYTE* m_charMapB = nullptr;
	};
}
