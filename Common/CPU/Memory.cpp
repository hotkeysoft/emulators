#include "stdafx.h"

#include <CPU/Memory.h>

namespace fs = std::filesystem;

namespace emul
{
	WORD Memory::s_uninitialized = 0xF00F;

	Memory::Memory(WORD blockGranularity) : Logger("MEM"), m_blockGranularity(blockGranularity)
	{
		assert(IsPowerOf2(blockGranularity));
		assert(blockGranularity >= 8);
		assert(blockGranularity <= 65536);
		MemoryBlockBase::SetBlockGranularity(blockGranularity);
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

		size_t memSlots = ((uint64_t)1 << addressBits) / m_blockGranularity;
		LogPrintf(LOG_INFO, "BlockGranularity: %d, Allocating %d memory slots", m_blockGranularity, memSlots);

		m_memory.reserve(memSlots);
		for (size_t i = 0; i < memSlots; ++i)
		{
			m_memory.push_back(MemorySlot());
		}
	}

	bool Memory::Allocate(MemoryBlockBase* block, ADDRESS base, DWORD len)
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

		if (len % m_blockGranularity != 0)
		{
			LogPrintf(LOG_ERROR, "Block size [%d] is not a multiple of [%d]", len, m_blockGranularity);
			return false;
		}

		if (!CheckAddressRange(base, m_addressBits) ||
			!CheckAddressRange(base + (DWORD)len - 1, m_addressBits))
		{
			LogPrintf(LOG_ERROR, "Address out of range: block at %X, size = %d bytes", base, len);
			LogPrintf(LOG_ERROR, "CPU Max address: %X", GetMaxAddress(m_addressBits));
			return false;
		}

		int nbSlots = len / m_blockGranularity;
		int minSlot = base / m_blockGranularity;
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

	bool Memory::Free(MemoryBlockBase* block)
	{
		LogPrintf(LOG_INFO, "Freeing block [%s]", block->GetId().c_str());

		for (size_t i = 0; i < m_memory.size(); ++i)
		{
			MemorySlot& slot = m_memory[i];
			MemoryBlockBase* oldBlock = slot.block;
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

		if (len % m_blockGranularity != 0)
		{
			LogPrintf(LOG_ERROR, "Window size [%d] is not a multiple of [%d]", len, m_blockGranularity);
			return false;
		}

		if (source % m_blockGranularity != 0)
		{
			LogPrintf(LOG_ERROR, "Source base address [%d] is not aligned to [%d] bytes boundary", source, m_blockGranularity);
			return false;
		}

		if (window % m_blockGranularity != 0)
		{
			LogPrintf(LOG_ERROR, "Window base address [%d] is not aligned to [%d] bytes boundary", window, m_blockGranularity);
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

		int nbSlots = len / m_blockGranularity;
		int sourceBaseSlot = source / m_blockGranularity;
		int windowBaseSlot = window / m_blockGranularity;
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
		const MemorySlot& slot = FindBlock(baseAddress);
		MemoryBlock* block = dynamic_cast<MemoryBlock*>(slot.block);

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
		const MemorySlot& slot = FindBlock(address);
		MemoryBlockBase* block = slot.block;

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
		const MemorySlot& slot = FindBlock(address);
		MemoryBlockBase* block = slot.block;

		if (block)
		{
			block->write(address - slot.base, value);
		}
		else
		{
			LogPrintf(LOG_WARNING, "Writing unallocated memory space (%X)", address);
		}
	}

	void Memory::Write16(ADDRESS address, WORD value)
	{
		Write8(address, GetLByte(value));
		Write8(address + 1, GetHByte(value));
	}

	void Memory::Dump(ADDRESS start, DWORD len, const char* outFile)
	{
		const MemorySlot& slot = FindBlock(start);
		MemoryBlock* block = dynamic_cast<MemoryBlock*>(slot.block);

		if (block)
		{
			block->Dump(start - slot.base, len, outFile);
		}
		else
		{
			LogPrintf(LOG_WARNING, "No block allocated at address (%X)", start);
		}
	}

	MemoryBlockBase* Memory::FindBlock(const char* id) const
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
		for (const auto rawBlock : m_blocks)
		{
			json blockJson;
			blockJson["size"] = rawBlock->GetSize();
			blockJson["type"] = rawBlock->GetType();

			// Dump only memory blocks, not io blocks
			MemoryBlock* block = dynamic_cast<MemoryBlock*>(rawBlock);
			if (block)
			{
				std::string fileName = "memory_" + block->GetId() + ".bin";
				fs::path path = GetSerializationDir();
				path.append(fileName);
				block->Dump(0, block->GetSize(), path.string().c_str());
				blockJson["file"] = fileName;
			}
			blocks[rawBlock->GetId()] = blockJson;

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
			if (type != MemoryType::IO)
			{
				std::string fileName = source["file"];

				MemoryBlock* dest = dynamic_cast<MemoryBlock*>(FindBlock(id.c_str()));
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
}