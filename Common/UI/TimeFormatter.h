#pragma once

#include <time.h>
#include <string>

namespace ui
{
	enum class TimeFormat
	{
		TF_ABSOLUTE,
		TF_RELATIVE
	};

	class TimeFormatter
	{
	public:
		TimeFormatter(time_t);

		std::string ToString(TimeFormat f = TimeFormat::TF_RELATIVE) const;

	protected:
		std::string FormatAbsolute() const;
		std::string FormatRelative() const;

		time_t m_time = 0;
	};
}
