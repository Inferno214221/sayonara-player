/* StringConverter.h */
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
#ifndef SAYONARA_PLAYER_STRINGCONVERTER_H
#define SAYONARA_PLAYER_STRINGCONVERTER_H

#include "Utils/Pimpl.h"

#include <memory>
#include <optional>

class QString;
namespace SmartPlaylists
{
	class StringConverter
	{
		public:
			virtual ~StringConverter();
			[[nodiscard]] virtual QString intToString(int value) const;
			[[nodiscard]] virtual QString intToUserString(int value) const;
			[[nodiscard]] virtual std::optional<int> stringToInt(const QString& str) const;
	};

	using StringConverterPtr = std::shared_ptr<StringConverter>;

} // SmartPlaylists

#endif //SAYONARA_PLAYER_STRINGCONVERTER_H
