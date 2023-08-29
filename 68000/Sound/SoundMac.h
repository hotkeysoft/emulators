#pragma once

#include <CPU/Memory.h>
#include <Logger.h>

namespace sound::mac
{
	class SoundMac
	{
	public:
		SoundMac(emul::Memory& memory) : m_memory(memory)
		{

		}
		virtual ~SoundMac() {}

		SoundMac(const SoundMac&) = delete;
		SoundMac& operator=(const SoundMac&) = delete;
		SoundMac(SoundMac&&) = delete;
		SoundMac& operator=(SoundMac&&) = delete;

		// TODO: Find better doc about sound module
		void Enable(bool enable)
		{
			m_enabled = enable;
			if (!m_enabled)
			{
				ResetBufferPos();
			}
		}

		void SetBufferBase(emul::ADDRESS base) { m_bufferBase = base; }
		void ResetBufferPos() { m_bufferPos = 0; }
		void BufferWord()
		{
			m_word = m_memory.Read16be(m_bufferBase + m_bufferPos);
			m_bufferPos += m_enabled ? 2 : 0;
		}

		emul::WORD GetBufferWord() const { return m_word; }

	protected:
		bool m_enabled = true;

		emul::WORD m_word = 0;

		emul::ADDRESS m_bufferBase = 0;
		emul::ADDRESS m_bufferPos = 0;

		emul::Memory& m_memory;
	};
}

