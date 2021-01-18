/* PlaylistDBInterface.cpp */

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

#include <Database/Connector.h>
#include "PlaylistDBWrapper.h"
#include "PlaylistDBInterface.h"

#include "Utils/Algorithm.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Language/Language.h"
#include "PlaylistChangeNotifier.h"

namespace Algorithm=Util::Algorithm;
using Playlist::DBInterface;
using Util::SaveAsAnswer;

struct DBInterface::Private
{
	Playlist::DBWrapper*  playlistDatabaseWrapper=nullptr;
	QString             name;
	bool                isTemporary;
	int                 id;

	Private(const QString& name) :
		name(name),
		isTemporary(true)
	{
		playlistDatabaseWrapper = new DBWrapper();
		id = playlistDatabaseWrapper->getPlaylistByName(name).id();
	}

	~Private()
	{
		delete playlistDatabaseWrapper;
		playlistDatabaseWrapper=nullptr;
	}
};

DBInterface::DBInterface(const QString& name)
{
	m = Pimpl::make<Private>(name);
}

DBInterface::~DBInterface() {}

int DBInterface::id() const
{
	return m->id;
}

void DBInterface::setId(int id)
{
	m->id = id;
}

QString DBInterface::name() const
{
	return m->name;
}

void DBInterface::setName(const QString& name)
{
	m->name = name;
}

bool DBInterface::isTemporary() const
{
	return m->isTemporary;
}

void DBInterface::setTemporary(bool b)
{
	m->isTemporary = b;
}

SaveAsAnswer DBInterface::save()
{
	if(m->id >= 0)
	{
		const auto success = m->playlistDatabaseWrapper->savePlaylist(tracks(), m->id, m->isTemporary);
		if(success) {
			this->setChanged(false);
			PlaylistChangeNotifier::instance()->addPlaylist(id(), name());
		}

		return success ? SaveAsAnswer::Success : SaveAsAnswer::OtherError;
	}

	else {
		const auto ret = saveAs(m->name, true);
		if(ret == SaveAsAnswer::Success)
		{
			PlaylistChangeNotifier::instance()->addPlaylist(id(), name());
		}

		return ret;
	}
}

bool DBInterface::insertTemporaryIntoDatabase()
{
	if(!m->isTemporary) {
		return false;
	}

	bool success = m->playlistDatabaseWrapper->savePlaylistTemporary(tracks(), m->name);
	if(success) {
		m->id = m->playlistDatabaseWrapper->getPlaylistByName(m->name).id();
	}

	return success;
}

SaveAsAnswer DBInterface::saveAs(const QString& name, bool forceOverride)
{
	if(name.isEmpty()) {
		return Util::SaveAsAnswer::InvalidName;
	}

	CustomPlaylistSkeletons skeletons;
	m->playlistDatabaseWrapper->getAllSkeletons(skeletons);

	// check if name already exists
	auto it = Util::Algorithm::find(skeletons, [&name](auto skeleton){
		return (name.compare(skeleton.name(), Qt::CaseInsensitive) == 0);
	});

	int targetId = -1;
	if(it != skeletons.end())
	{
		if(!forceOverride) {
			return SaveAsAnswer::NameAlreadyThere;
		}

		targetId = it->id();
	}

	// Name already exists, override
	bool success;
	if(targetId >= 0){
		success = m->playlistDatabaseWrapper->savePlaylist(this->tracks(), targetId, m->isTemporary);
	}

	// New playlist
	else
	{
		success = m->playlistDatabaseWrapper->savePlaylistAs(this->tracks(), name);

		if(success && this->isTemporary())
		{
			const auto oldId = this->id();
			m->playlistDatabaseWrapper->deletePlaylist(oldId);
		}
	}

	if(success)
	{
		const auto id = m->playlistDatabaseWrapper->getPlaylistByName(name).id();
		if(id >= 0){
			this->setId(id);
		}

		this->setTemporary(false);
		this->setName(name);
		this->setChanged(false);

		PlaylistChangeNotifier::instance()->addPlaylist(id, name);

		return SaveAsAnswer::Success;
	}

	return SaveAsAnswer::OtherError;
}

SaveAsAnswer DBInterface::rename(const QString& newName)
{
	auto oldName = this->name();
	if(newName.isEmpty()) {
		return Util::SaveAsAnswer::InvalidName;
	}

	CustomPlaylistSkeletons skeletons;
	m->playlistDatabaseWrapper->getAllSkeletons(skeletons);

	// check if name already exists
	bool exists = Util::Algorithm::contains(skeletons, [&newName](auto skeleton){
		return (newName.compare(skeleton.name(), Qt::CaseInsensitive) == 0);
	});

	if(exists){
		return SaveAsAnswer::NameAlreadyThere;
	}

	bool success = m->playlistDatabaseWrapper->renamePlaylist(m->id, newName);
	if(success){
		this->setName(newName);
		PlaylistChangeNotifier::instance()->renamePlaylist(id(), oldName, newName);
		return SaveAsAnswer::Success;
	}

	return SaveAsAnswer::OtherError;
}

bool DBInterface::deletePlaylist()
{
	bool success = (m->id >= 0)
		? m->playlistDatabaseWrapper->deletePlaylist(m->id)
		: m->playlistDatabaseWrapper->deletePlaylist(m->name);

	m->isTemporary = true;
	return success;
}

QString DBInterface::requestNewDatabaseName(QString prefix)
{
	if(prefix.isEmpty())
	{
		prefix = Lang::get(Lang::New);
	}

	CustomPlaylistSkeletons skeletons;

	auto pdw = std::make_shared<DBWrapper>();
	pdw->getAllSkeletons(skeletons);

	QString target_name;

	for(int idx = 1; idx < 1000; idx++)
	{
		bool found = false;
		target_name = prefix + " " + QString::number(idx);
		for(const CustomPlaylistSkeleton& skeleton : Algorithm::AsConst(skeletons))
		{
			QString name = skeleton.name();

			if(name.compare(target_name, Qt::CaseInsensitive) == 0){
				found = true;
				break;
			}
		}

		if(!found){
			break;
		}
	}

	return target_name;
}

MetaDataList DBInterface::fetchTracksFromDatabase() const
{
	return m->playlistDatabaseWrapper->getPlaylistById(this->id());
}