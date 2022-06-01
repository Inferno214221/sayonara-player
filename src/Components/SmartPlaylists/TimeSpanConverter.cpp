/* TimeSpanConverter.cpp */
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

#include "TimeSpanConverter.h"

#include <QDate>
#include <QObject>
#include <QRegExp>
#include <QStringList>

namespace SmartPlaylists
{
	namespace
	{
		constexpr const auto DaysPerYear = 365;
		constexpr const auto DaysPerMonth = 30;
	}

	QString TimeSpanConverter::intToString(int value) const
	{
		if(value == 0)
		{
			return {"0"};
		}

		auto result = QStringList();

		if(const auto years = (value / DaysPerYear); years > 0)
		{
			result << QObject::tr("%ny", "", years);
		}

		if(const auto months = (value % DaysPerYear) / DaysPerMonth; months > 0)
		{
			result << QObject::tr("%nm", "", months);
		}

		if(const auto days = (value % DaysPerYear) % DaysPerMonth; days > 0)
		{
			result << QObject::tr("%nd", "", days);
		}

		return result.join(", ");
	}

	std::optional<int> TimeSpanConverter::stringToInt(const QString& str) const
	{
		if(str.isEmpty())
		{
			return std::nullopt;
		}
		
		constexpr const auto* reText =
			R"(^((\d)y,?\s*)?((\d+)m,?\s*)?((\d+)d?)?$)";

		const auto re = QRegExp(reText);
		if(const auto index = re.indexIn(str); index >= 0)
		{
			const auto yearStr = re.cap(2);
			const auto monthStr = re.cap(4);
			const auto dayStr = re.cap(6);
			const auto capturedTexts = re.capturedTexts();
			const auto result = yearStr.toInt() * DaysPerYear +
			                    monthStr.toInt() * DaysPerMonth +
			                    dayStr.toInt();

			return result;
		}

		return StringConverter::stringToInt(str);
	}
}