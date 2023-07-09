#pragma once

#include <type_traits>
#include <string>
#include <cassert>

namespace hscommon::bitUtil
{
template <typename T,
	typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
class BitMask
{
public:
	BitMask(T mask = 0, T maskBits = 0) { Set(mask, maskBits); }
	BitMask(const char* maskStr) { Set(maskStr); }

	void Clear() { Set(0, 0); }

	T GetMask() const { return m_mask; }
	T GetMaskBits() const { return m_maskBits; }

	void Set(T mask, T maskBits = 0)
	{
		m_maskBits = maskBits ? maskBits : mask;
		m_mask = mask & m_maskBits;
	};

	bool Set(const char* maskStr)
	{
		constexpr size_t bits = (sizeof(T) * 8);

		Clear();
		if (!maskStr || !maskStr[0] || (strlen(maskStr) > bits))
		{
			assert(false);
			return false;
		}

		const char* pos = maskStr;
		while(true)
		{
			switch (*pos)
			{
			case '0':
				m_mask &= ~1;
				m_maskBits |= 1;
				break;
			case '1':
				m_mask |= 1;
				m_maskBits |= 1;
				break;
			case 'x':
			case 'X':
				m_maskBits &= ~1;
				break;

			default:
				assert(false);
				Clear();
				return false;
			}

			++pos;
			if (*pos)
			{
				m_mask <<= 1;
				m_maskBits <<= 1;
			}
			else
			{
				break;
			}
		}

		return true;
	};

	bool IsSet() const { return m_maskBits != 0; }

	bool IsMatch(T value) const { return (value & m_maskBits) == m_mask; }

	std::string ToString() const
	{
		std::ostringstream os;
		T mask = m_mask;
		T maskBits = m_maskBits;
		constexpr size_t bits = (sizeof(T) * 8);

		for (size_t bit = 0; bit < bits; ++bit) {
			os << (GetHiBit(maskBits) ? (GetHiBit(mask) ? '1' : '0') : 'x');
			maskBits <<= 1;
			mask <<= 1;
		}

		return os.str();
	}

protected:
	static bool GetHiBit(T val) { return val & (1 << ((sizeof(T) * 8) - 1)); }

	T m_mask = 0;
	T m_maskBits = 0;
};

}