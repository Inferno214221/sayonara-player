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
#include "SmartPlaylistRandomTracks.h"
#include "SmartPlaylistRandomAlbum.h"

#include "Components/Session/Session.h"
#include "Database/SmartPlaylists.h"
#include "Utils/Algorithm.h"
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

	QList<int> splitAttributes(const QString& attribute)
	{
		const auto splitted = attribute.split(",");
		if(splitted.size() < 1)
		{
			throw WrongAttributeException {attribute.toStdString()};
		}

		auto result = QList<int> {};
		Util::Algorithm::transform(splitted, result, [](const auto& str) {
			return str.toInt();
		});

		return result;
	}
}

std::shared_ptr<SmartPlaylist> SmartPlaylists::create(const SmartPlaylistDatabaseEntry& entry)
{
	try
	{
		const auto values = splitAttributes(entry.attributes);

		if(entry.classType == SmartPlaylistByRating::ClassType)
		{
			return createFromType(SmartPlaylists::Type::Rating, entry.id, values, entry.isRandomized);
		}

		if(entry.classType == SmartPlaylistByYear::ClassType)
		{
			return createFromType(SmartPlaylists::Type::Year, entry.id, values, entry.isRandomized);
		}

		if(entry.classType == SmartPlaylistByCreateDate::ClassType)
		{
			return createFromType(SmartPlaylists::Type::Created, entry.id, values, entry.isRandomized);
		}

		if(entry.classType == SmartPlaylistByRelativeDate::ClassType)
		{
			return createFromType(SmartPlaylists::Type::CreatedRelative, entry.id, values, entry.isRandomized);
		}

		if(entry.classType == SmartPlaylistByListeningDate::ClassType)
		{
			return createFromType(SmartPlaylists::Type::LastPlayed, entry.id, values, entry.isRandomized);
		}

		if(entry.classType == SmartPlaylistRandomTracks::ClassType)
		{
			return createFromType(SmartPlaylists::Type::RandomTracks, entry.id, values, entry.isRandomized);
		}

		if(entry.classType == SmartPlaylistRandomAlbum::ClassType)
		{
			return createFromType(SmartPlaylists::Type::RandomAlbums, entry.id, values, entry.isRandomized);
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
SmartPlaylists::createFromType(const SmartPlaylists::Type field, const int id, const QList<int>& values,
                               const bool isRandomized)
{
	using SmartPlaylists::Type;
	switch(field)
	{
		case Type::Rating:
			return std::make_shared<SmartPlaylistByRating>(id, values[0], values[1], isRandomized);
		case Type::Year:
			return std::make_shared<SmartPlaylistByYear>(id, values[0], values[1], isRandomized);
		case Type::Created:
			return std::make_shared<SmartPlaylistByCreateDate>(id, values[0], values[1], isRandomized);
		case Type::CreatedRelative:
			return std::make_shared<SmartPlaylistByRelativeDate>(id, values[0], values[1], isRandomized);
		case Type::LastPlayed:
			return std::make_shared<SmartPlaylistByListeningDate>(id, values[0], values[1], isRandomized);
		case Type::RandomTracks:
			return std::make_shared<SmartPlaylistRandomTracks>(id, values[0]);
		case Type::RandomAlbums:
			return std::make_shared<SmartPlaylistRandomAlbum>(id, values[0], isRandomized);
		default:
			return nullptr;
	}
}
