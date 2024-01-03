/* SmartPlaylistManager.cpp */
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

#include "SmartPlaylistManager.h"
#include "SmartPlaylistByRating.h"
#include "SmartPlaylistCreator.h"

#include "Database/Connector.h"
#include "Database/Library.h"
#include "Database/LibraryDatabase.h"
#include "Database/SmartPlaylists.h"
#include "Interfaces/PlaylistInterface.h"
#include "Utils/FileSystem.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/RandomGenerator.h"

struct SmartPlaylistManager::Private
{
	PlaylistCreator* playlistCreator;
	Util::FileSystemPtr fileSystem;
	std::map<Spid, SmartPlaylistPtr> smartPlaylists;

	Private(PlaylistCreator* playlistCreator, Util::FileSystemPtr fileSystem_) :
		playlistCreator {playlistCreator},
		fileSystem {std::move(fileSystem_)}
	{
		auto* db = DB::Connector::instance()->smartPlaylistsConnector();
		const auto smartPlaylistDatabaseEntries = db->getAllSmartPlaylists();
		for(const auto& entry: smartPlaylistDatabaseEntries)
		{
			const auto smartPlaylist = SmartPlaylists::create(entry, fileSystem);
			if(smartPlaylist)
			{
				smartPlaylists[Spid(smartPlaylist->id())] = smartPlaylist;
			}
		}
	}
};

SmartPlaylistManager::SmartPlaylistManager(PlaylistCreator* playlistCreator, const Util::FileSystemPtr& fileSystem) :
	m {Pimpl::make<Private>(playlistCreator, fileSystem)} {}

SmartPlaylistManager::~SmartPlaylistManager() = default;

SmartPlaylistPtr SmartPlaylistManager::smartPlaylist(const Spid& id) const
{
	return m->smartPlaylists[id];
}

QList<SmartPlaylistPtr> SmartPlaylistManager::smartPlaylists() const
{
	QList<SmartPlaylistPtr> smartPlaylists;
	for(const auto& [key, value]: m->smartPlaylists)
	{
		smartPlaylists << value;
	}

	return smartPlaylists;
}

void SmartPlaylistManager::selectPlaylist(const Spid& id)
{
	const auto smartPlaylist = m->smartPlaylists[id];
	auto tracks = MetaDataList {};

	if(!smartPlaylist->canFetchTracks())
	{
		const auto* libraryDatabase = DB::Connector::instance()->libraryDatabase(smartPlaylist->libraryId(), 0);
		libraryDatabase->getAllTracks(tracks);
	}

	auto filteredTracks = smartPlaylist->filterTracks(std::move(tracks));
	if(smartPlaylist->isRandomized())
	{
		Util::Algorithm::shuffle(filteredTracks);
	}

	m->playlistCreator->createPlaylist(filteredTracks, smartPlaylist->name(), true);
}

void SmartPlaylistManager::deletePlaylist(const Spid& id)
{
	auto* db = DB::Connector::instance()->smartPlaylistsConnector();
	const auto success = db->deleteSmartPlaylist(m->smartPlaylists[id]->id());
	if(success)
	{
		m->smartPlaylists.erase(id);
		emit sigPlaylistsChanged();
	}
}

void SmartPlaylistManager::insertPlaylist(const SmartPlaylistPtr& smartPlaylist)
{
	auto* db = DB::Connector::instance()->smartPlaylistsConnector();
	const auto id = db->insertSmartPlaylist(smartPlaylist->toDatabaseEntry());
	if(id >= 0)
	{
		smartPlaylist->setId(id);
		m->smartPlaylists[Spid(smartPlaylist->id())] = smartPlaylist;
		emit sigPlaylistsChanged();
	}
}

void SmartPlaylistManager::updatePlaylist(const Spid& id, const SmartPlaylistPtr& smartPlaylist)
{
	auto* db = DB::Connector::instance()->smartPlaylistsConnector();
	const auto success = db->updateSmartPlaylist(id.id, smartPlaylist->toDatabaseEntry());
	if(success)
	{
		m->smartPlaylists[id] = smartPlaylist;
		emit sigPlaylistsChanged();
	}
}

SmartPlaylistPtr SmartPlaylistManager::createAndInsert(SmartPlaylists::Type field, int id, const QList<int>& values,
                                                       const bool isRandomized, const LibraryId libraryId)
{
	auto smartPlaylist = SmartPlaylists::createFromType(field, id, values, isRandomized, libraryId, m->fileSystem);
	insertPlaylist(smartPlaylist);

	return smartPlaylist;
}
