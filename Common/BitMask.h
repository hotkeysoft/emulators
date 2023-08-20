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
		// maskBits: '1' bit indicate exact bit match
		//           '0' bit indicate don't care ('x')
		//
		// maskBits == '111...111': IsMatch will only return true when value == mask
		//                          (exact match, default if not set)
		// maskBits == '000...000': IsMatch will always return true (mask == 'xxx...xxx')
		BitMask() { Clear(); }
		BitMask(T mask, T maskBits = static_cast<T>(-1)) { Set(mask, maskBits); }
		BitMask(const char* maskStr) { Set(maskStr); }

		bool IsSet() const { return m_set; }
		operator bool() const { return IsSet(); }

		void Clear() { Set(0, 0); m_set = false; }

		T GetMask() const { return m_mask; }
		T GetMaskBits() const { return m_maskBits; }

		void Set(T mask, T maskBits = static_cast<T>(-1))
		{
			m_set = true;
			m_maskBits = maskBits;
			m_mask = mask & m_maskBits; // Clear 'don't care' bits in mask
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
			while (true)
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

			m_set = true;
			return true;
		};

		// Returns a BitMask with merged values
		// Takes original mask and override with all non-'x' bits of mergeWith
		BitMask Merge(const BitMask& mergeWith) const
		{
			BitMask ret(*this);
			ret.m_mask &= ~mergeWith.m_maskBits; // Clear the non-'x' bits
			ret.m_mask |= mergeWith.m_mask;
			ret.m_maskBits |= mergeWith.m_maskBits;
			return ret;
		}

		bool IsMatch(T value) const { return (value & m_maskBits) == m_mask; }

		// Applies this mask on top of a value.
		// This will set all the 0/1 bits in the mask, and leave the 'x' bits alone.
		T Apply(T src) const { return (src & (~m_maskBits)) | m_mask; }

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
		bool m_set = false;
	};

}