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

	bool Memory::AllocateOffset(MemoryBlockBase* block, ADDRESS sourceOffset, ADDRESS base, DWORD len, AllocateMode mode)
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

		if ((sourceOffset + len) > block->GetSize())
		{
			LogPrintf(LOG_ERROR, "Requested size [%d] is larger than source block", len);
			return false;
		}

		if (!CheckAddressRange(base, m_addressBits) ||
			!CheckAddressRange(base + (DWORD)len - 1, m_addressBits))
		{
			LogPrintf(LOG_ERROR, "Address out of range: block at %X, size = %d bytes", base, len);
			LogPrintf(LOG_ERROR, "CPU Max address: %X", GetMaxAddress(m_addressBits));
			return false;
		}

		size_t nbSlots = len / m_blockGranularity;
		size_t minSlot = base / m_blockGranularity;
		LogPrintf(LOG_DEBUG, "Using %d slots, first slot = %02Xh", nbSlots, minSlot);

		base -= sourceOffset;

		for (size_t i = 0; i < nbSlots; ++i)
		{
			MemorySlot& slot = m_memory[minSlot + i];

			switch (mode)
			{
			case AllocateMode::READ:
				if (slot.blockR)
				{
					LogPrintf(LOG_INFO, "Slot [%x] already allocated for READ to [%s], replacing with block [%s]", i,
						slot.blockR->GetId().c_str(),
						block->GetId().c_str());
				}
				slot.blockR = block;
				slot.baseR = base;
				break;
			case AllocateMode::WRITE:
				if (slot.blockW)
				{
					LogPrintf(LOG_INFO, "Slot [%x] already allocated for WRITE to [%s], replacing with block [%s]", i,
						slot.blockW->GetId().c_str(),
						block->GetId().c_str());
				}
				slot.blockW = block;
				slot.baseW = base;
				break;
			case AllocateMode::READ_WRITE:
				if (slot.blockR || slot.blockW)
				{
					// Lazy, get either one
					const emul::MemoryBlockBase* oldBlock = slot.blockW ? slot.blockW : slot.blockR;
					LogPrintf(LOG_INFO, "Slot [%x] already allocated for READ|WRITE to [%s], replacing with block [%s]", i,
						oldBlock->GetId().c_str(),
						block->GetId().c_str());
				}
				slot.blockR = block;
				slot.blockW = block;
				slot.baseR = base;
				slot.baseW = base;
				break;
			default:
				throw std::exception("not possible");
			}
		}

		m_blocks.insert(block);
		return true;
	}

	bool Memory::Restore(ADDRESS base, DWORD len, AllocateMode mode)
	{
		if (m_addressBits == 0)
		{
			LogPrintf(LOG_ERROR, "Not Initialized");
			return false;
		}

		if ((mode != AllocateMode::READ) && (mode != AllocateMode::WRITE))
		{
			LogPrintf(LOG_ERROR, "Invalid AllocateMode (only READ|WRITE allowed)");
			return false;
		}
		LogPrintf(LOG_INFO, "Restore [%s] @ %X, size = %d bytes", (mode == AllocateMode::READ) ? "READ" : "WRITE", base, len);

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

		size_t nbSlots = len / m_blockGranularity;
		size_t minSlot = base / m_blockGranularity;

		for (size_t i = 0; i < nbSlots; ++i)
		{
			MemorySlot& slot = m_memory[minSlot + i];

			if (mode == AllocateMode::READ)
			{
				slot.blockR = slot.blockW;
				slot.baseR = slot.baseW;
			}
			else
			{
				slot.blockW = slot.blockR;
				slot.baseW = slot.baseR;
			}
		}

		return true;
	}

	bool Memory::Free(MemoryBlockBase* block)
	{
		LogPrintf(LOG_INFO, "Freeing block [%s]", block->GetId().c_str());

		for (size_t i = 0; i < m_memory.size(); ++i)
		{
			MemorySlot& slot = m_memory[i];

			if (block == slot.blockR)
			{
				slot.blockR = nullptr;
				slot.baseR = 0;
			}

			if (block == slot.blockW)
			{
				slot.blockW = nullptr;
				slot.baseW = 0;
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

		size_t nbSlots = len / m_blockGranularity;
		size_t sourceBaseSlot = source / m_blockGranularity;
		size_t windowBaseSlot = window / m_blockGranularity;
		LogPrintf(LOG_DEBUG, "Copying %d slots, base slot = %02Xh -> %02Xh", nbSlots, sourceBaseSlot, windowBaseSlot);

		for (size_t i = 0; i < nbSlots; ++i)
		{
			MemorySlot slot = m_memory[sourceBaseSlot + i];
			slot.baseR += (window - source);
			slot.baseW += (window - source);

			m_memory[windowBaseSlot + i] = slot;
		}

		return true;
	}

	bool Memory::LoadBinary(const char* file, ADDRESS baseAddress)
	{
		const MemorySlot& slot = FindBlock(baseAddress);
		MemoryBlock* block = dynamic_cast<MemoryBlock*>(slot.blockR);

		if (!block)
		{
			// TODO: Create block
			LogPrintf(LOG_ERROR, "LoadBinary: No memory allocated at address %04X", baseAddress);
			return false;
		}
		return block->LoadFromFile(file, baseAddress - slot.baseR);
	}

	BYTE Memory::Read8(ADDRESS address) const
	{
		address &= m_addressMask;
		const MemorySlot& slot = FindBlock(address);
		const MemoryBlockBase* block = slot.blockR;

		if (block)
		{
			return block->read(address - slot.baseR);
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

	WORD Memory::Read16be(ADDRESS address) const
	{
		BYTE h = Read8(address);
		BYTE l = Read8(address + 1);
		return MakeWord(h, l);
	}

	DWORD Memory::Read32be(ADDRESS address) const
	{
		WORD h = Read16be(address);
		WORD l = Read16be(address + 2);
		return MakeDword(h, l);
	}

	void Memory::Write8(ADDRESS address, BYTE value)
	{
		address &= m_addressMask;
		const MemorySlot& slot = FindBlock(address);
		MemoryBlockBase* block = slot.blockW;

		if (block)
		{
			block->write(address - slot.baseW, value);
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

	void Memory::Write16be(ADDRESS address, WORD value)
	{
		Write8(address, GetHByte(value));
		Write8(address + 1, GetLByte(value));
	}

	void Memory::Write32be(ADDRESS address, DWORD value)
	{
		Write16be(address, GetHWord(value));
		Write16be(address + 2, GetLWord(value));
	}

	void Memory::Dump(ADDRESS start, DWORD len, const char* outFile)
	{
		const MemorySlot& slot = FindBlock(start);
		MemoryBlock* block = dynamic_cast<MemoryBlock*>(slot.blockW);

		if (block)
		{
			block->Dump(start - slot.baseW, len, outFile);
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

	bool Memory::FillRAM(ADDRESS baseAddress, const MemoryBlock::RawBlock& data)
	{
		WORD bytesToLoad = (WORD)data.size();

		// minimum check: check if baseAddress hits RAM
		const MemorySlot& slot = FindBlock(baseAddress);
		MemoryBlock* targetBlock = dynamic_cast<MemoryBlock*>(slot.blockW);
		if (!targetBlock)
		{
			LogPrintf(LOG_ERROR, "FillRAM: No memory allocated at load address: %04X", baseAddress);
			return false;
		}
		else if (targetBlock->GetType() != MemoryType::RAM)
		{
			LogPrintf(LOG_ERROR, "FillRAM: No RAM allocated at load address: %04X", baseAddress);
			return false;
		}

		// Lazy way to do it
		for (BYTE b : data)
		{
			Write8(baseAddress++, b);
		}
		return true;
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