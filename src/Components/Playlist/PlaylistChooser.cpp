/* PlaylistChooser.cpp */

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
	int						importState;

	Handler*		playlistHandler=nullptr;
	DBWrapperPtr	playlistDbConnector=nullptr;

	Private()
	{
		playlistHandler = Handler::instance();
		playlistDbConnector = std::make_shared<DBWrapper>();
	}

	CustomPlaylist findCustomPlaylist(int id)
	{
		CustomPlaylist pl = playlistDbConnector->getPlaylistById(id);
		return pl;
	}
};

Chooser::Chooser(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	m->playlistDbConnector->getNonTemporarySkeletons(
		m->skeletons, SortOrder::NameAsc
	);

	PlaylistChangeNotifier* pcn = PlaylistChangeNotifier::instance();

	connect(pcn, &PlaylistChangeNotifier::sigPlaylistAdded, this, &Chooser::playlistAdded);
	connect(pcn, &PlaylistChangeNotifier::sigPlaylistRenamed, this, &Chooser::playlistRenamed);
	connect(pcn, &PlaylistChangeNotifier::sigPlaylistDeleted, this, &Chooser::playlistDeleted);
}

Chooser::~Chooser() = default;

const CustomPlaylistSkeletons& Chooser::playlists()
{
	return m->skeletons;
}

Util::SaveAsAnswer Chooser::renamePlaylist(int id, const QString& new_name)
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
		spLog(Log::Warning, this) << "Playlist " << new_name << " exists already";
		return SaveAsAnswer::NameAlreadyThere;
	}

	bool success = m->playlistDbConnector->renamePlaylist(id, new_name);
	if(success)	{
		PlaylistChangeNotifier::instance()->renamePlaylist(id, old_name, new_name);
		return SaveAsAnswer::Success;
	}

	return SaveAsAnswer::OtherError;
}

bool Chooser::deletePlaylist(int id)
{
	if(id < 0){
		return false;
	}

	bool success = m->playlistDbConnector->deletePlaylist(id);
	if(success)	{
		PlaylistChangeNotifier::instance()->deletePlaylist(id);
	}

	return success;
}

void Chooser::loadSinglePlaylist(int id)
{
	if(id < 0) {
		return;
	}

	CustomPlaylist pl = m->findCustomPlaylist(id);

	int idx = m->playlistHandler->createPlaylist(pl);
	m->playlistHandler->set_current_index(idx);
}


int Chooser::findPlaylist(const QString& name) const
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

void Chooser::playlistsChanged()
{
	m->skeletons.clear();
	m->playlistDbConnector->getNonTemporarySkeletons(
		m->skeletons, SortOrder::NameAsc
	);

	emit sigPlaylistsChanged();
}


void Chooser::playlistDeleted(int id)
{
	Q_UNUSED(id)

	playlistsChanged();
}

void Chooser::playlistAdded(int id, const QString& name)
{
	Q_UNUSED(id)
	Q_UNUSED(name)

	playlistsChanged();
}

void Chooser::playlistRenamed(int id, const QString& old_name, const QString& new_name)
{
	Q_UNUSED(id)
	Q_UNUSED(old_name)
	Q_UNUSED(new_name)

	playlistsChanged();
}
