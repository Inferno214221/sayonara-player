/* DateConverter.h */
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
#ifndef SAYONARA_PLAYER_DATECONVERTER_H
#define SAYONARA_PLAYER_DATECONVERTER_H

#include "StringConverter.h"

class QDate;
namespace SmartPlaylists
{
	class DateConverter :
		public StringConverter
	{
		public:
			[[nodiscard]] QString intToString(int value) const override;
			[[nodiscard]] std::optional<int> stringToInt(const QString& str) const override;
	};

	[[nodiscard]] int dateToInt(const QDate& date);
	[[nodiscard]] QDate intToDate(int value);

} // SmartPlaylists

#endif //SAYONARA_PLAYER_DATECONVERTER_H
