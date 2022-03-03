#pragma once
#include "../CPU/MemoryBlock.h"
#include "../Serializable.h"

#include <array>

namespace graph_ega
{
	struct GraphControllerData;
}

namespace seq_ega
{
	struct SequencerData;
}

namespace memory_ega
{
	enum class RAMSIZE { EGA_64K = 65536, EGA_128K = 131072, EGA_256K = 262144 };

	class MemoryEGA : public emul::MemoryBlock, public emul::Serializable
	{
	public:
		MemoryEGA(RAMSIZE ramsize);
		virtual ~MemoryEGA() {}

		virtual void Clear(BYTE filler = 0) override;

		void SetGraphController(const graph_ega::GraphControllerData* graph) { m_graphData = graph; }
		void SetSequencer(const seq_ega::SequencerData* seq) { m_seqData = seq; }

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
		virtual void Deserialize(json& from) override;

	protected:
		RAMSIZE m_ramSize;
		bool m_enable = false;

		const graph_ega::GraphControllerData* m_graphData = nullptr;
		const seq_ega::SequencerData* m_seqData = nullptr;

		const emul::DWORD m_planeSize;
		const emul::ADDRESS m_planeAddressMask;
		
		MemoryBlock m_planes[4];
		std::array<BYTE, 4> m_dataLatches = { 0, 0, 0, 0 };

		const BYTE* m_charMapA = nullptr;
		const BYTE* m_charMapB = nullptr;
	};
}
