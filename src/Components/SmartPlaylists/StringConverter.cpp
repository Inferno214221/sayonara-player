/* StringConverter.cpp */
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

#include "StringConverter.h"

#include <QString>

namespace SmartPlaylists
{
	StringConverter::~StringConverter() = default;

	QString StringConverter::intToString(const int value) const { return QString::number(value); }

	std::optional<int> StringConverter::stringToInt(const QString& str) const
	{
		auto ok = false;
		const auto number = str.toInt(&ok);
		return ok
		       ? std::optional {number}
		       : std::nullopt;
	}
}