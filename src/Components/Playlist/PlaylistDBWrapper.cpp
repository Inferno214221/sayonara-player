/* PlaylistDBWrapper.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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
	DB::Playlist* db=nullptr;

	Private()
	{
		db = DB::Connector::instance()->playlist_connector();
	}
};

DBWrapper::DBWrapper()
{
	m = Pimpl::make<Private>();
}


DBWrapper::~DBWrapper() {}


void DBWrapper::apply_tags(MetaDataList& v_md)
{
	for(MetaData& md : v_md)
	{
		if(md.is_extern)
		{
			if(Util::File::is_file(md.filepath())){
				Tagging::Utils::getMetaDataOfFile(md);
			}
		}
	}
}


bool DBWrapper::get_skeletons(CustomPlaylistSkeletons& skeletons, Playlist::StoreType type, Playlist::SortOrder so){
	return m->db->getAllPlaylistSkeletons(skeletons, type, so);
}

bool DBWrapper::get_all_skeletons(CustomPlaylistSkeletons& skeletons,
					   Playlist::SortOrder so)
{
	return get_skeletons(skeletons,
						 Playlist::StoreType::TemporaryAndPermanent,
						 so);
}

bool DBWrapper::get_temporary_skeletons(CustomPlaylistSkeletons& skeletons,
												Playlist::SortOrder so)
{
	return get_skeletons(skeletons,
						 Playlist::StoreType::OnlyTemporary,
						 so);
}

bool DBWrapper:: get_non_temporary_skeletons(CustomPlaylistSkeletons& skeletons,
													 Playlist::SortOrder so)
{
	return get_skeletons(skeletons,
						 Playlist::StoreType::OnlyPermanent,
						 so);
}


bool DBWrapper::get_playlists(CustomPlaylists& playlists, Playlist::StoreType type, Playlist::SortOrder so)
{
	Q_UNUSED(type)

	bool success;
	CustomPlaylistSkeletons skeletons;

	success = get_all_skeletons(skeletons, so);
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

		success = m->db->getPlaylistById(pl);

		if(!success){
			continue;
		}

		apply_tags(pl);

		if( (pl.temporary() && load_temporary) ||
			(!pl.temporary() && load_permanent) )
		{
			playlists.push_back(pl);
		}
	}

	return true;
}


bool DBWrapper::get_all_playlists(CustomPlaylists& playlists, Playlist::SortOrder so)
{
	return get_playlists(playlists,
						 Playlist::StoreType::TemporaryAndPermanent,
						 so);
}


bool DBWrapper::get_temporary_playlists(CustomPlaylists& playlists, Playlist::SortOrder so)
{
	return get_playlists(playlists,
						 Playlist::StoreType::OnlyTemporary,
						 so);
}


bool DBWrapper::get_non_temporary_playlists(CustomPlaylists& playlists, Playlist::SortOrder so)
{
	return get_playlists(playlists,
						 Playlist::StoreType::OnlyPermanent,
						 so);
}


CustomPlaylist DBWrapper::get_playlist_by_id(int id)
{
	bool success;
	CustomPlaylist pl;
	pl.set_id(id);

	success = m->db->getPlaylistById(pl);
	if(!success){
		return pl;
	}

	return pl;
}


CustomPlaylist DBWrapper::get_playlist_by_name(const QString& name)
{
	int id = m->db->getPlaylistIdByName(name);

	if(id < 0){
		CustomPlaylist pl;
		pl.set_id(-1);
		return pl;
	}

	return get_playlist_by_id(id);
}

bool DBWrapper::rename_playlist(int id, const QString& new_name)
{
	return m->db->renamePlaylist(id, new_name);
}


bool DBWrapper::save_playlist_as(const MetaDataList& v_md, const QString& name)
{
	DB::Connector* db = DB::Connector::instance();

	db->transaction();
	bool success = m->db->storePlaylist(v_md, name, false);
	db->commit();

	return success;
}

bool DBWrapper::save_playlist_temporary(const MetaDataList& v_md, const QString& name)
{
	DB::Connector* db = DB::Connector::instance();

	db->transaction();

	bool success = m->db->storePlaylist(v_md, name, true);

	db->commit();

	return success;
}


bool DBWrapper::save_playlist(const CustomPlaylist& pl)
{
	DB::Connector* db = DB::Connector::instance();

	db->transaction();
	// TODO! we dont need the two other parameters
	bool success = m->db->storePlaylist(pl, pl.id(), pl.temporary());
	db->commit();

	return success;
}


bool DBWrapper::save_playlist(const MetaDataList& v_md, int id, bool is_temporary)
{
	DB::Connector* db = DB::Connector::instance();

	db->transaction();
	// TODO: see above
	bool success = m->db->storePlaylist(v_md, id, is_temporary);
	db->commit();

	return success;
}


bool DBWrapper::delete_playlist(int id)
{
	return m->db->deletePlaylist(id);
}


bool DBWrapper::delete_playlist(const QString& name)
{
	int id = m->db->getPlaylistIdByName(name);
	return m->db->deletePlaylist(id);
}


bool DBWrapper::exists(const QString& name)
{
	int id = m->db->getPlaylistIdByName(name);
	return (id >= 0);
}
