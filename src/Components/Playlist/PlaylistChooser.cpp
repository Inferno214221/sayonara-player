/* PlaylistChooser.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "PlaylistChooser.h"
#include "PlaylistChangeNotifier.h"

#include "Utils/Playlist/CustomPlaylist.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/PlaylistDBWrapper.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Playlist/Sorting.h"

#include <QStringList>

namespace Algorithm=Util::Algorithm;

using Playlist::Handler;
using Playlist::Chooser;
using Playlist::DBWrapper;


struct Chooser::Private
{
	CustomPlaylistSkeletons	skeletons;
	int						import_state;

	Handler*		playlist_handler=nullptr;
	DBWrapperPtr	playlist_db_connector=nullptr;

	Private()
	{
		playlist_handler = Handler::instance();
		playlist_db_connector = std::make_shared<DBWrapper>();
	}

	CustomPlaylist find_custom_playlist(int id)
	{
		CustomPlaylist pl = playlist_db_connector->get_playlist_by_id(id);
		return pl;
	}
};

Chooser::Chooser(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	m->playlist_db_connector->get_non_temporary_skeletons(
		m->skeletons, SortOrder::NameAsc
	);

	PlaylistChangeNotifier* pcn = PlaylistChangeNotifier::instance();

	connect(pcn, &PlaylistChangeNotifier::sig_playlist_added, this, &Chooser::playlist_added);
	connect(pcn, &PlaylistChangeNotifier::sig_playlist_renamed, this, &Chooser::playlist_renamed);
	connect(pcn, &PlaylistChangeNotifier::sig_playlist_deleted, this, &Chooser::playlist_deleted);
}

Chooser::~Chooser() = default;

const CustomPlaylistSkeletons& Chooser::playlists()
{
	return m->skeletons;
}

Util::SaveAsAnswer Chooser::rename_playlist(int id, const QString& new_name)
{
	using Util::SaveAsAnswer;

	if(new_name.isEmpty()) {
		return SaveAsAnswer::InvalidName;
	}

	if(id < 0) {
		return SaveAsAnswer::InvalidObject;
	}

	bool exists = false;
	QString old_name;
	for(auto it=m->skeletons.begin(); it != m->skeletons.end(); it++)
	{
		if(it->name() == new_name){
			exists = true;
		}

		if(it->id() == id) {
			old_name = it->name();
		}
	}

	if(exists) {
		sp_log(Log::Warning, this) << "Playlist " << new_name << " exists already";
		return SaveAsAnswer::NameAlreadyThere;
	}

	bool success = m->playlist_db_connector->rename_playlist(id, new_name);
	if(success)	{
		PlaylistChangeNotifier::instance()->rename_playlist(id, old_name, new_name);
		return SaveAsAnswer::Success;
	}

	return SaveAsAnswer::OtherError;
}

bool Chooser::delete_playlist(int id)
{
	if(id < 0){
		return false;
	}

	bool success = m->playlist_db_connector->delete_playlist(id);
	if(success)	{
		PlaylistChangeNotifier::instance()->delete_playlist(id);
	}

	return success;
}

void Chooser::load_single_playlist(int id)
{
	if(id < 0) {
		return;
	}

	CustomPlaylist pl = m->find_custom_playlist(id);

	int idx = m->playlist_handler->create_playlist(pl);
	m->playlist_handler->set_current_index(idx);
}


int Chooser::find_playlist(const QString& name) const
{
	for(const CustomPlaylistSkeleton& skeleton : Algorithm::AsConst(m->skeletons))
	{
		if(skeleton.name().compare(name) == 0)
		{
			return skeleton.id();
		}
	}

	return -1;
}

void Chooser::playlists_changed()
{
	m->skeletons.clear();
	m->playlist_db_connector->get_non_temporary_skeletons(
		m->skeletons, SortOrder::NameAsc
	);

	emit sig_playlists_changed();
}


void Chooser::playlist_deleted(int id)
{
	Q_UNUSED(id)

	playlists_changed();
}

void Chooser::playlist_added(int id, const QString& name)
{
	Q_UNUSED(id)
	Q_UNUSED(name)

	playlists_changed();
}

void Chooser::playlist_renamed(int id, const QString& old_name, const QString& new_name)
{
	Q_UNUSED(id)
	Q_UNUSED(old_name)
	Q_UNUSED(new_name)

	playlists_changed();
}
