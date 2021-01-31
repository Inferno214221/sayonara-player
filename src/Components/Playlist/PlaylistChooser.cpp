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

#include "Interfaces/PlaylistInterface.h"
#include "Components/Playlist/PlaylistDBWrapper.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Playlist/Sorting.h"
#include "Utils/Playlist/CustomPlaylist.h"

#include <QStringList>

namespace Algorithm = Util::Algorithm;

using Playlist::Chooser;
using Playlist::DBWrapper;

struct Chooser::Private
{
	CustomPlaylistSkeletons skeletons;
	PlaylistCreator* playlistCreator = nullptr;
	DBWrapperPtr playlistDbConnector = nullptr;

	Private(PlaylistCreator* playlistCreator) :
		playlistCreator {playlistCreator},
		playlistDbConnector {std::make_shared<DBWrapper>()}
	{
		playlistDbConnector->getNonTemporarySkeletons(skeletons, SortOrder::NameAsc);
	}
};

Chooser::Chooser(PlaylistCreator* playlistCreator, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(playlistCreator);

	auto* pcn = PlaylistChangeNotifier::instance();
	connect(pcn, &PlaylistChangeNotifier::sigPlaylistAdded, this, &Chooser::playlistAdded);
	connect(pcn, &PlaylistChangeNotifier::sigPlaylistRenamed, this, &Chooser::playlistRenamed);
	connect(pcn, &PlaylistChangeNotifier::sigPlaylistDeleted, this, &Chooser::playlistDeleted);
}

Chooser::~Chooser() = default;

const CustomPlaylistSkeletons& Chooser::playlists()
{
	return m->skeletons;
}

Util::SaveAsAnswer Chooser::renamePlaylist(int id, const QString& newName)
{
	using Util::SaveAsAnswer;

	if(newName.isEmpty())
	{
		return SaveAsAnswer::InvalidName;
	}

	if(id < 0)
	{
		return SaveAsAnswer::InvalidObject;
	}

	auto nameExists = false;
	QString oldName;
	for(auto it = m->skeletons.begin(); it != m->skeletons.end(); it++)
	{
		if(it->name() == newName)
		{
			nameExists = true;
		}

		if(it->id() == id)
		{
			oldName = it->name();
		}
	}

	if(nameExists)
	{
		spLog(Log::Warning, this) << "Playlist " << newName << " exists already";
		return SaveAsAnswer::NameAlreadyThere;
	}

	const auto success = m->playlistDbConnector->renamePlaylist(id, newName);
	if(success)
	{
		PlaylistChangeNotifier::instance()->renamePlaylist(id, oldName, newName);
		return SaveAsAnswer::Success;
	}

	return SaveAsAnswer::OtherError;
}

bool Chooser::deletePlaylist(int id)
{
	if(id >= 0)
	{
		const auto success = m->playlistDbConnector->deletePlaylist(id);
		if(success)
		{
			PlaylistChangeNotifier::instance()->deletePlaylist(id);
		}

		return success;
	}

	return false;
}

void Chooser::loadSinglePlaylist(int id)
{
	if(id >= 0)
	{
		const auto pl = m->playlistDbConnector->getPlaylistById(id);
		m->playlistCreator->createPlaylist(pl);
	}
}

int Chooser::findPlaylist(const QString& name) const
{
	const auto it = Util::Algorithm::find(m->skeletons, [&name](const auto& skeleton) {
		return (skeleton.name() == name);
	});

	return (it != m->skeletons.end()) ? it->id() : -1;
}

void Chooser::playlistsChanged()
{
	m->skeletons.clear();
	m->playlistDbConnector->getNonTemporarySkeletons(
		m->skeletons, SortOrder::NameAsc
	);

	emit sigPlaylistsChanged();
}

void Chooser::playlistDeleted(int /*id*/)
{
	playlistsChanged();
}

void Chooser::playlistAdded(int /*id*/, const QString& /*name*/)
{
	playlistsChanged();
}

void Chooser::playlistRenamed(int /*id*/, const QString& /*oldName*/, const QString& /*newName*/)
{
	playlistsChanged();
}
