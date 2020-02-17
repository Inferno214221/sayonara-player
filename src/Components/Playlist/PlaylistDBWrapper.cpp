/* PlaylistDBWrapper.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "PlaylistDBWrapper.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Tagging/Tagging.h"

#include "Database/Connector.h"
#include "Database/Playlist.h"

#include <utility>

namespace Algorithm=Util::Algorithm;
using Playlist::DBWrapper;

struct DBWrapper::Private
{
	DB::Playlist* playlistDatabase=nullptr;
};

DBWrapper::DBWrapper()
{
	m = Pimpl::make<Private>();
	m->playlistDatabase = DB::Connector::instance()->playlistConnector();
}

DBWrapper::~DBWrapper() = default;

void DBWrapper::applyTags(MetaDataList& v_md)
{
	for(MetaData& md : v_md)
	{
		if(md.isExtern())
		{
			if(Util::File::isFile(md.filepath())){
				Tagging::Utils::getMetaDataOfFile(md);
			}
		}
	}
}


bool DBWrapper::getSkeletons(CustomPlaylistSkeletons& skeletons, Playlist::StoreType type, Playlist::SortOrder so)
{
	return m->playlistDatabase->getAllPlaylistSkeletons(skeletons, type, so);
}

bool DBWrapper::getAllSkeletons(CustomPlaylistSkeletons& skeletons,
					   Playlist::SortOrder so)
{
	return getSkeletons(skeletons,
						 Playlist::StoreType::TemporaryAndPermanent,
						 so);
}

bool DBWrapper::getTemporarySkeletons(CustomPlaylistSkeletons& skeletons,
												Playlist::SortOrder so)
{
	return getSkeletons(skeletons,
						 Playlist::StoreType::OnlyTemporary,
						 so);
}

bool DBWrapper:: getNonTemporarySkeletons(CustomPlaylistSkeletons& skeletons,
													 Playlist::SortOrder so)
{
	return getSkeletons(skeletons,
						 Playlist::StoreType::OnlyPermanent,
						 so);
}


bool DBWrapper::getPlaylists(CustomPlaylists& playlists, Playlist::StoreType type, Playlist::SortOrder so)
{
	Q_UNUSED(type)

	bool success;
	CustomPlaylistSkeletons skeletons;

	success = getAllSkeletons(skeletons, so);
	if(!success){
		return false;
	}

	bool load_temporary = (type == Playlist::StoreType::OnlyTemporary ||
						   type == Playlist::StoreType::TemporaryAndPermanent);

	bool load_permanent = (type == Playlist::StoreType::OnlyPermanent ||
						   type == Playlist::StoreType::TemporaryAndPermanent);

	for(const CustomPlaylistSkeleton& skeleton : Algorithm::AsConst(skeletons))
	{
		CustomPlaylist pl(skeleton);
		if(pl.id() < 0){
			continue;
		}

		success = m->playlistDatabase->getPlaylistById(pl);

		if(!success){
			continue;
		}

		applyTags(pl);

		if( (pl.temporary() && load_temporary) ||
			(!pl.temporary() && load_permanent) )
		{
			playlists.push_back(pl);
		}
	}

	return true;
}


bool DBWrapper::getAllPlaylists(CustomPlaylists& playlists, Playlist::SortOrder so)
{
	return getPlaylists(playlists,
						 Playlist::StoreType::TemporaryAndPermanent,
						 so);
}


bool DBWrapper::getTemporaryPlaylists(CustomPlaylists& playlists, Playlist::SortOrder so)
{
	return getPlaylists(playlists,
						 Playlist::StoreType::OnlyTemporary,
						 so);
}


bool DBWrapper::getNonTemporaryPlaylists(CustomPlaylists& playlists, Playlist::SortOrder so)
{
	return getPlaylists(playlists,
						 Playlist::StoreType::OnlyPermanent,
						 so);
}


CustomPlaylist DBWrapper::getPlaylistById(int id)
{
	bool success;
	CustomPlaylist pl;
	pl.setId(id);

	success = m->playlistDatabase->getPlaylistById(pl);
	if(!success){
		return pl;
	}

	return pl;
}


CustomPlaylist DBWrapper::getPlaylistByName(const QString& name)
{
	int id = m->playlistDatabase->getPlaylistIdByName(name);

	if(id < 0){
		CustomPlaylist pl;
		pl.setId(-1);
		return pl;
	}

	return getPlaylistById(id);
}

bool DBWrapper::renamePlaylist(int id, const QString& new_name)
{
	return m->playlistDatabase->renamePlaylist(id, new_name);
}


bool DBWrapper::savePlaylistAs(const MetaDataList& v_md, const QString& name)
{
	auto* db = DB::Connector::instance();

	db->transaction();
	bool success = m->playlistDatabase->storePlaylist(v_md, name, false);
	db->commit();

	return success;
}

bool DBWrapper::savePlaylistTemporary(const MetaDataList& v_md, const QString& name)
{
	auto* db = DB::Connector::instance();

	db->transaction();

	bool success = m->playlistDatabase->storePlaylist(v_md, name, true);

	db->commit();

	return success;
}


bool DBWrapper::savePlaylist(const CustomPlaylist& pl)
{
	auto* db = DB::Connector::instance();

	db->transaction();
	// TODO! we dont need the two other parameters
	bool success = m->playlistDatabase->storePlaylist(pl, pl.id(), pl.temporary());
	db->commit();

	return success;
}


bool DBWrapper::savePlaylist(const MetaDataList& v_md, int id, bool is_temporary)
{
	auto* db = DB::Connector::instance();

	db->transaction();
	// TODO: see above
	bool success = m->playlistDatabase->storePlaylist(v_md, id, is_temporary);
	db->commit();

	return success;
}


bool DBWrapper::deletePlaylist(int id)
{
	return m->playlistDatabase->deletePlaylist(id);
}


bool DBWrapper::deletePlaylist(const QString& name)
{
	int id = m->playlistDatabase->getPlaylistIdByName(name);
	return m->playlistDatabase->deletePlaylist(id);
}


bool DBWrapper::exists(const QString& name)
{
	int id = m->playlistDatabase->getPlaylistIdByName(name);
	return (id >= 0);
}

