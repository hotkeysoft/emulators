#include "stdafx.h"

#include "TimeFormatter.h"
#include <tuple>
#include <vector>

namespace ui
{
	const int SecondsInMinute = 60;
	const int SecondsInHour = SecondsInMinute * 60;
	const int SecondsInDay = SecondsInHour * 24;
	const int SecondsInWeek = SecondsInDay * 7;
	const int SecondsInMonth = SecondsInDay * 31;
	const int SecondsInYear = SecondsInDay * 365;

	using TimeInterval = std::tuple<int, std::string>;
	using TimeIntervals = std::vector<TimeInterval>;

	const TimeIntervals intervals = {
		{ SecondsInMinute, "second" },
		{ SecondsInHour, "minute" },
		{ SecondsInDay, "hour" },
		{ SecondsInWeek, "day" },
		{ SecondsInMonth, "week" },
		{ SecondsInYear, "month" },
		{ std::numeric_limits<int>::max(), "year"}
	};

	TimeFormatter::TimeFormatter(time_t t) : m_time(t)
	{
	}

	std::string TimeFormatter::FormatAbsolute() const
	{
		if (m_time == 0)
		{
			return "(Invalid date)";
		}

		char buf[128];
		struct tm* local = localtime(&m_time);
		strftime(buf, sizeof(buf), "%c", local);
		return buf;
	}

	std::string TimeFormatter::FormatRelative() const
	{
		if (m_time == 0)
		{
			return "(Invalid date)";
		}

		std::ostringstream os;

		time_t now = time(nullptr);
		time_t delta = now - m_time;

		bool first = true;
		int lastDivider = 1;
		for (const auto& interval : intervals)
		{
			if (delta < std::get<0>(interval))
			{
				if (first)
				{
					return "Just now";
				}
				else
				{
					first = false;
					auto count = delta / lastDivider;
					os << count << " " << std::get<1>(interval)
						<< ((count > 1) ? "s " : " ")
						<< "ago";
					break;
				}
			}
			lastDivider = std::get<0>(interval);
			first = false;
		}
		return os.str();
	}

	std::string TimeFormatter::ToString(TimeFormat f) const
	{
		switch (f)
		{
		case TimeFormat::TF_RELATIVE:
			return FormatRelative();
		default:
		case TimeFormat::TF_ABSOLUTE:
			return FormatAbsolute();
		}
	}
}
