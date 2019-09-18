#include "stdafx.h"
#include "MemoryBlock.h"
#include <memory.h>

MemoryBlock::MemoryBlock(WORD baseAddress, WORD size, MemoryType type)
	:	m_baseAddress(baseAddress),
		m_size(size),
		m_type(type)
{
	m_invalid = 0xFA;

	m_data = new BYTE[m_size];

	Clear();
}

MemoryBlock::MemoryBlock(WORD baseAddress, const std::vector<BYTE>data, MemoryType type /*=RAM*/) :	
	m_baseAddress(baseAddress),
	m_type(type)
{
	if (data.size() ==0 || data.size() > 0xFFFF)
		throw std::exception("Invalid block size");
	m_size = (WORD)data.size();

	m_invalid = 0xFA;

	m_data = new BYTE[m_size];

	for (int i=0; i<m_size; i++)
		m_data[i] = data[i];
}

MemoryBlock::MemoryBlock(const MemoryBlock &block) :
	m_baseAddress(block.m_baseAddress),
	m_size(block.m_size),
	m_invalid(block.m_invalid),
	m_type(block.m_type)
{
	m_data = new BYTE[m_size];
	memcpy(m_data, block.m_data, m_size);
}

MemoryBlock::~MemoryBlock()
{
	delete [] m_data;
}

void MemoryBlock::Clear(BYTE filler)
{
	memset(m_data, filler, m_size);
}

BYTE MemoryBlock::read(WORD address)
{
	if (address < m_baseAddress || address >= m_baseAddress + m_size)
		return m_invalid;
	else
		return m_data[address-m_baseAddress];	
}

void MemoryBlock::write(WORD address,char data)
{
	if (address < m_baseAddress || address >= m_baseAddress + m_size)
		return;

	m_data[address-m_baseAddress] = data;
}
