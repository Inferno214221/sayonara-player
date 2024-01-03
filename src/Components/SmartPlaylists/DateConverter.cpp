/* DateConverter.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "DateConverter.h"

#include <QDate>
#include <QLocale>
#include <QString>

namespace SmartPlaylists
{
	int dateToInt(const QDate& date)
	{
		return date.year() * 10'000 + // NOLINT(readability-magic-numbers)
		       date.month() * 100 + // NOLINT(readability-magic-numbers)
		       date.day();
	}

	QDate intToDate(const int value)
	{
		if((value > 21000000) || (value < 15000101))
		{
			return QDate {};
		}

		const auto year = value / 10'000; // NOLINT(readability-magic-numbers)
		const auto month = (value % 10'000) / 100; // NOLINT(readability-magic-numbers)
		const auto day = value % 100; // NOLINT(readability-magic-numbers)

		return {year, month, day};
	}

	QString DateConverter::intToString(const int value) const
	{
		const auto date = intToDate(value);
		return date.toString("yyyy-MM-dd");
	}

	QString DateConverter::intToUserString(const int value) const
	{
		const auto date = intToDate(value);
		return date.toString(QLocale().dateFormat(QLocale::FormatType::ShortFormat));
	}

	std::optional<int> DateConverter::stringToInt(const QString& str) const
	{
		const auto date = QDate::fromString(str, "yyyy-MM-dd");
		return date.isValid()
		       ? dateToInt(date)
		       : std::optional<int> {};
	}
}