#include "CartridgePCjr.h"

using emul::DWORD;

namespace cart
{
	CartridgePCjr::CartridgePCjr() : MemoryBlock("cartPCjr", emul::MemoryType::ROM)
	{
	}

	bool CartridgePCjr::LoadFromFile(const char* file, WORD offset)
	{
		if (offset)
		{
			LogPrintf(LOG_ERROR, "LoadCartridge: offset argument not used for cartridges");
			return false;
		}
		LogPrintf(LOG_INFO, "LoadCartridge: loading %s", file);

		FILE* f = fopen(file, "rb");
		if (!f)
		{
			LogPrintf(LOG_ERROR, "LoadCartridge: error opening binary file");
			return false;
		}

		if (fread(&m_header, sizeof(m_header), 1, f) != 1)
		{
			LogPrintf(LOG_ERROR, "LoadCartridge: error reading header");
			return false;
		}

		// Do some validations
		if (memcmp(m_header.signature, expectedSignature, sizeof(m_header.signature)) != 0)
		{
			LogPrintf(LOG_ERROR, "LoadCartridge: unexpected signature [%s]", m_header.signature);
			return false;
		}
		if (m_header.versionH != 1 || m_header.versionL != 0)
		{
			LogPrintf(LOG_WARNING, "LoadCartridge: unexpected version [%d.%d]", m_header.versionH, m_header.versionL);
		}
		if (m_header.adrmask != 0)
		{
			LogPrintf(LOG_WARNING, "LoadCartridge: unsupported address mask [%04Xh]", m_header.adrmask);
		}

		LogPrintf(LOG_INFO, "LoadCartridge: base address: [%04Xh]", m_header.address);

		// Determine ROM size
		fseek(f, 0L, SEEK_END);
		DWORD end = ftell(f);

		fseek(f, sizeof(m_header), SEEK_SET);
		DWORD curr = ftell(f);

		DWORD size = end - curr;

		if (!emul::IsPowerOf2((DWORD)size))
		{
			LogPrintf(LOG_WARNING, "LoadCartridge: size is not a power of two: [%d]", size);
		}

		m_size = RoundBlockSize(size);

		if (size != m_size)
		{
			LogPrintf(LOG_WARNING, "LoadCartridge: block size inflated to: [%d]", m_size);
		}

		delete[] m_data;
		m_data = new BYTE[m_size];

		size_t bytesRead = fread(m_data, sizeof(char), size, f);
		if (bytesRead < 1)
		{
			LogPrintf(LOG_ERROR, "LoadCartridge: error reading binary file");
			return false;
		}
		else
		{
			LogPrintf(LOG_INFO, "LoadCartridge: read %d bytes to memory block", bytesRead);
		}

		fclose(f);
		return true;
	}
}