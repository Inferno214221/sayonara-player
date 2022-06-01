/* SmartPlaylistCreator.cpp */
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

#include "SmartPlaylistCreator.h"
#include "SmartPlaylistByRating.h"
#include "SmartPlaylistByYear.h"
#include "SmartPlaylistByCreateDate.h"
#include "SmartPlaylistByRelativeDate.h"
#include "SmartPlaylistByListeningDate.h"

#include "Components/Session/Session.h"

#include "Database/SmartPlaylists.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>
#include <stdexcept>

namespace
{
	class WrongAttributeException :
		std::invalid_argument
	{
		public:
			explicit WrongAttributeException(const std::string& exceptionString) :
				invalid_argument(std::string {"Wrong attribute"}.append(": ").append(exceptionString)) {}
	};

	std::pair<int, int> splitAttributes(const QString& attribute)
	{
		const auto splitted = attribute.split(",");
		if(splitted.size() != 2)
		{
			throw WrongAttributeException {attribute.toStdString()};
		}

		return {
			splitted[0].toInt(),
			splitted[1].toInt()
		};
	}
}

std::shared_ptr<SmartPlaylist> SmartPlaylists::create(const SmartPlaylistDatabaseEntry& entry)
{
	try
	{
		const auto params = splitAttributes(entry.attributes);

		if(entry.classType == SmartPlaylistByRating::ClassType)
		{
			return createFromType(SmartPlaylists::Type::Rating, entry.id, params.first, params.second);
		}

		if(entry.classType == SmartPlaylistByYear::ClassType)
		{
			return createFromType(SmartPlaylists::Type::Year, entry.id, params.first, params.second);
		}

		if(entry.classType == SmartPlaylistByCreateDate::ClassType)
		{
			return createFromType(SmartPlaylists::Type::Created, entry.id, params.first, params.second);
		}

		if(entry.classType == SmartPlaylistByRelativeDate::ClassType)
		{
			return createFromType(SmartPlaylists::Type::CreatedRelative, entry.id, params.first, params.second);
		}

		if(entry.classType == SmartPlaylistByListeningDate::ClassType)
		{
			return createFromType(SmartPlaylists::Type::LastPlayed, entry.id, params.first, params.second);
		}

		return nullptr;
	}

	catch(std::exception& e)
	{
		spLog(Log::Error, "SmartPlaylistCreator") << e.what();
		return nullptr;
	}
}

std::shared_ptr<SmartPlaylist>
SmartPlaylists::createFromType(const SmartPlaylists::Type field, const int id, const int min, const int max)
{
	using SmartPlaylists::Type;
	switch(field)
	{
		case Type::Rating:
			return std::make_shared<SmartPlaylistByRating>(id, min, max);
		case Type::Year:
			return std::make_shared<SmartPlaylistByYear>(id, min, max);
		case Type::Created:
			return std::make_shared<SmartPlaylistByCreateDate>(id, min, max);
		case Type::CreatedRelative:
			return std::make_shared<SmartPlaylistByRelativeDate>(id, min, max);
		case Type::LastPlayed:
			return std::make_shared<SmartPlaylistByListeningDate>(id, min, max);
		default:
			return nullptr;
	}
}
