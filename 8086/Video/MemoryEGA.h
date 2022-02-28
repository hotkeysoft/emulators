#pragma once
#include "../CPU/MemoryBlock.h"

namespace graph_ega
{
	class GraphController;
}

namespace memory_ega
{
	enum class RAMSIZE { EGA_64K = 65536, EGA_128K = 131072, EGA_256K = 262144 };

	class MemoryEGA : public emul::MemoryBlock
	{
	public:
		MemoryEGA(RAMSIZE ramsize);
		virtual ~MemoryEGA() {}

		virtual void Clear(BYTE filler = 0) override;

		void SetGraphController(graph_ega::GraphController* ctrl) { m_graphCtrl = ctrl; }

		void Enable(bool enable) { m_enable = enable; }

		virtual BYTE read(emul::ADDRESS offset) override;
		virtual void write(emul::ADDRESS offset, BYTE data) override;

		// TODO: Temporary direct access to ram for video card drawing
		BYTE readRaw(BYTE plane, emul::ADDRESS offset) { return m_planes[plane].read(offset); }

		// Character Maps
		void SelectCharMaps(BYTE selectA, BYTE selectB);

		void SetPlaneMask(BYTE mask) { m_planeMask = mask & 0x0F; }

		const BYTE* GetCharMapB() const { return m_charMapA; }
		const BYTE* GetCharMapA() const { return m_charMapB; }

		virtual bool LoadFromFile(const char* file, WORD offset = 0) override;
		virtual bool Dump(emul::ADDRESS offset, emul::DWORD len, const char* outFile) const override;

	protected:
		RAMSIZE m_ramSize;
		bool m_enable = false;

		graph_ega::GraphController* m_graphCtrl = nullptr;

		const emul::DWORD m_planeSize;
		const emul::ADDRESS m_planeAddressMask;
		
		MemoryBlock m_planes[4];
		BYTE m_planeMask = 0x0F;
		BYTE m_dataLatches[4] = { 0, 0, 0, 0 };

		const BYTE* m_charMapA = nullptr;
		const BYTE* m_charMapB = nullptr;
	};
}
