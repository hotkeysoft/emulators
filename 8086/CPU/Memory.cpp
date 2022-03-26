#include "stdafx.h"

#include "Memory.h"

namespace fs = std::filesystem;

namespace emul
{
	WORD Memory::s_uninitialized = 0xF00F;

	const WORD BlockGranularity = 4096;

	Memory::Memory() : Logger("MEM")	
	{
	}

	Memory::~Memory()
	{
	}

	void Memory::Init(size_t addressBits)
	{
		m_addressBits = addressBits;
		m_addressMask = (ADDRESS)GetMaxAddress(addressBits);

		LogPrintf(LOG_INFO, "Setting up [%d] bit memory address space mask=["PRINTF_BIN_PATTERN_INT32"]", 
			addressBits, 
			PRINTF_BYTE_TO_BIN_INT32(m_addressMask));

		size_t memSlots = ((uint64_t)1 << addressBits) / BlockGranularity;
		LogPrintf(LOG_INFO, "BlockGranularity: %d, Allocating %d memory slots", BlockGranularity, memSlots);

		m_memory.reserve(memSlots);
		for (size_t i = 0; i < memSlots; ++i)
		{
			m_memory.push_back(MemorySlot());
		}
	}

	bool Memory::Allocate(MemoryBlock* block, ADDRESS base, DWORD len)
	{
		assert(block);

		if (m_addressBits == 0)
		{
			LogPrintf(LOG_ERROR, "Not Initialized");
			return false;
		}

		if (len == (DWORD)-1)
		{
			len = block->GetSize();
		}
		
		LogPrintf(LOG_INFO, "Request to allocate block [%s] at %X, size = %d bytes", block->GetId().c_str(), base, block->GetSize());

		if (len % BlockGranularity != 0)
		{
			LogPrintf(LOG_ERROR, "Block size [%d] is not a multiple of [%d]", len, BlockGranularity);
			return false;
		}

		if (!CheckAddressRange(base, m_addressBits) ||
			!CheckAddressRange(base + (DWORD)len - 1, m_addressBits))
		{
			LogPrintf(LOG_ERROR, "Address out of range: block at %X, size = %d bytes", base, len);
			LogPrintf(LOG_ERROR, "CPU Max address: %X", GetMaxAddress(m_addressBits));
			return false;
		}

		int nbSlots = len / BlockGranularity;
		int minSlot = base / BlockGranularity;
		LogPrintf(LOG_DEBUG, "Using %d slots, first slot = %02Xh", nbSlots, minSlot);

		for (size_t i = 0; i < nbSlots; ++i)
		{
			MemorySlot& slot = m_memory[minSlot + i];
			if (slot.block)
			{
				LogPrintf(LOG_WARNING, "Slot [%x] already allocated to [%s], replacing with block [%s]", i,
					slot.block->GetId().c_str(),
					block->GetId().c_str());
			}
			m_memory[minSlot + i] = {block, base};
		}

		m_blocks.insert(block);
		return true;
	}

	bool Memory::Free(MemoryBlock* block)
	{
		LogPrintf(LOG_INFO, "Freeing block [%s]", block->GetId().c_str());

		for (size_t i = 0; i < m_memory.size(); ++i)
		{
			MemorySlot& slot = m_memory[i];
			MemoryBlock* oldBlock = slot.block;
			if (oldBlock == block)
			{
				slot = { nullptr, 0 };
			}
		}

		m_blocks.erase(block);
		return true;
	}

	void Memory::Clear(BYTE filler)
	{
		for (auto block : m_blocks)
		{
			if (block->GetType() == MemoryType::RAM)
			{
				block->Clear(filler);
			}
		}
	}

	bool Memory::MapWindow(ADDRESS source, ADDRESS window, DWORD len)
	{
		LogPrintf(LOG_INFO, "Request to map memory [%X]-[%X] to window [%X]-[%X], window size: %d bytes",
			source, source + len - 1,
			window, window + len - 1,
			len);

		if (len % BlockGranularity != 0)
		{
			LogPrintf(LOG_ERROR, "Window size [%d] is not a multiple of [%d]", len, BlockGranularity);
			return false;
		}

		if (source % BlockGranularity != 0)
		{
			LogPrintf(LOG_ERROR, "Source base address [%d] is not aligned to [%d] bytes boundary", source, BlockGranularity);
			return false;
		}

		if (window % BlockGranularity != 0)
		{
			LogPrintf(LOG_ERROR, "Window base address [%d] is not aligned to [%d] bytes boundary", window, BlockGranularity);
			return false;
		}

		if (!CheckAddressRange(source, m_addressBits) ||
			!CheckAddressRange(source + len - 1, m_addressBits))
		{
			LogPrintf(LOG_ERROR, "Address out of range: block at %X, size = %d bytes", source, len);
			LogPrintf(LOG_ERROR, "CPU Max address: %X", GetMaxAddress(m_addressBits));
			return false;
		}

		if (!CheckAddressRange(window, m_addressBits) ||
			!CheckAddressRange(window + len - 1, m_addressBits))
		{
			LogPrintf(LOG_ERROR, "Address out of range: block at %X, size = %d bytes", source, len);
			LogPrintf(LOG_ERROR, "CPU Max address: %X", GetMaxAddress(m_addressBits));
			return false;
		}

		int nbSlots = len / BlockGranularity;
		int sourceBaseSlot = source / BlockGranularity;
		int windowBaseSlot = window / BlockGranularity;
		LogPrintf(LOG_DEBUG, "Copying %d slots, base slot = %02Xh -> %02Xh", nbSlots, sourceBaseSlot, windowBaseSlot);

		for (size_t i = 0; i < nbSlots; ++i)
		{			
			MemorySlot slot = m_memory[sourceBaseSlot + i];
			slot.base += (window - source);

			m_memory[windowBaseSlot + i] = slot;
		}

		return true;
	}

	bool Memory::LoadBinary(const char* file, ADDRESS baseAddress)
	{
		MemorySlot& slot = m_memory[baseAddress / BlockGranularity];
		MemoryBlock* block = slot.block;

		if (!block)
		{
			// TODO: Create block
			LogPrintf(LOG_ERROR, "LoadBinary: No memory allocated at address %04X", baseAddress);
			return false;
		}
		return block->LoadFromFile(file, baseAddress - slot.base);
	}

	BYTE Memory::Read8(ADDRESS address) const
	{
		address &= m_addressMask;
		const MemorySlot& slot = m_memory[address / BlockGranularity];
		MemoryBlock* block = slot.block;

		if (block)
		{
			return block->read(address - slot.base);
		}
		else
		{
			LogPrintf(LOG_WARNING, "Reading unallocated memory space (%X)", address);
			return 0x55;
		}
	}

	WORD Memory::Read16(ADDRESS address) const
	{
		BYTE l = Read8(address);
		BYTE h = Read8(address + 1);
		return MakeWord(h, l);
	}

	void Memory::Write8(ADDRESS address, BYTE value)
	{
		address &= m_addressMask;
		const MemorySlot& slot = m_memory[address / BlockGranularity];
		MemoryBlock* block = slot.block;

		if (block)
		{
			block->write(address - slot.base, value);
		}
		else
		{
			LogPrintf(LOG_WARNING, "Writing unallocated memory space (%X)", address);
		}
	}

	void Memory::Dump(ADDRESS start, DWORD len, const char* outFile)
	{
		MemorySlot& slot = m_memory[start / BlockGranularity];
		MemoryBlock* block = slot.block;

		if (block)
		{
			block->Dump(start - slot.base, len, outFile);
		}
		else
		{
			LogPrintf(LOG_WARNING, "No block allocated at address (%X)", start);
		}
	}

	MemoryBlock* Memory::FindBlock(const char* id) const
	{
		for (const auto block : m_blocks)
		{
			if (block->GetId() == id)
			{
				return block;
			}
		}
		return nullptr;
	}

	void Memory::Serialize(json& to)
	{
		json blocks;
		for (const auto block : m_blocks)
		{
			json blockJson;
			blockJson["size"] = block->GetSize();
			blockJson["type"] = block->GetType();

			std::string fileName = "memory_" + block->GetId() + ".bin";
			fs::path path = GetSerializationDir();
			path.append(fileName);
			block->Dump(0, block->GetSize(), path.string().c_str());
			blockJson["file"] = fileName;

			blocks[block->GetId()] = blockJson;

		}
		to["blocks"] = blocks;
	}

	void Memory::Deserialize(const json& from)
	{
		json blocks = from["blocks"];
		for (auto& block : blocks.items())
		{
			
			std::string id = block.key();
			json source = block.value();
			DWORD size = source["size"];
			MemoryType type = source["type"];
			std::string fileName = source["file"];

			MemoryBlock* dest = FindBlock(id.c_str());
			if (!dest)
			{
				LogPrintf(LOG_ERROR, "Deserialize: Block not found [%s]", id.c_str());
				return;
			}
			else if (size != dest->GetSize())
			{
				LogPrintf(LOG_ERROR, "Deserialize: Block size mismatch [%s]", id.c_str());
				return;
			}
			else if (type != dest->GetType())
			{
				LogPrintf(LOG_ERROR, "Deserialize: Block type mismatch [%s]", id.c_str());
				return;
			}

			fs::path path = GetSerializationDir();
			path.append(fileName);
			dest->LoadFromFile(path.string().c_str());
		}
	}
}