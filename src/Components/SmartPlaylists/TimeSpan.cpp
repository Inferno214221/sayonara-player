/* ${CLASS_NAME}.h */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TimeSpan.h"
#include "TimeSpanConverter.h"

#include <QStringList>
#include <QObject>

namespace SmartPlaylists
{
	namespace
	{
		constexpr const auto DaysPerYear = 360;
		constexpr const auto DaysPerMonth = 30;
	}

	QString timeSpanToString(const TimeSpan& timeSpan)
	{
		const int value = timeSpanToDays(timeSpan);
		const auto newTimeSpan = timeSpanFromDays(value);

		if(value == 0)
		{
			return {"0"};
		}

		auto result = QStringList();

		if(newTimeSpan.years > 0)
		{
			result << QObject::tr("%n year(s)", "", newTimeSpan.years);
		}

		if(newTimeSpan.months > 0)
		{
			result << QObject::tr("%n month(s)", "", newTimeSpan.months);
		}

		if(newTimeSpan.days > 0)
		{
			result << QObject::tr("%n days(s)", "", newTimeSpan.days);
		}

		return result.join(", ");
	}

	TimeSpan timeSpanFromDays(const int days)
	{
		return {
			days / DaysPerYear,
			(days % DaysPerYear) / DaysPerMonth,
			(days % DaysPerYear) % DaysPerMonth
		};
	}

	int timeSpanToDays(const TimeSpan& timeSpan)
	{
		return timeSpan.years * DaysPerYear +
		       timeSpan.months * DaysPerMonth +
		       timeSpan.days;
	}
}

