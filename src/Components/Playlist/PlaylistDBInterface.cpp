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

#include "PlaylistDBWrapper.h"
#include "PlaylistDBInterface.h"

#include "Utils/Algorithm.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Language/Language.h"

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
		bool success = m->playlistDatabaseWrapper->savePlaylist(tracks(), m->id, m->isTemporary);
		if(success) {
			this->setChanged(false);
			return SaveAsAnswer::Success;
		}

		else {
			return SaveAsAnswer::OtherError;
		}
	}

	else {
		return saveAs(m->name, true);
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

SaveAsAnswer DBInterface::saveAs(const QString& name, bool force_override)
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

	int tgt_id = -1;
	if(it != skeletons.end())
	{
		if(!force_override) {
			return SaveAsAnswer::NameAlreadyThere;
		}

		tgt_id = it->id();
	}

	// Name already exists, override
	bool success;
	if(tgt_id >= 0){
		success = m->playlistDatabaseWrapper->savePlaylist(this->tracks(), tgt_id, m->isTemporary);
	}

	// New playlist
	else
	{
		success = m->playlistDatabaseWrapper->savePlaylistAs(this->tracks(), name);

		if(success && this->isTemporary())
		{
			int old_id = this->id();
			m->playlistDatabaseWrapper->deletePlaylist(old_id);
		}
	}

	if(success)
	{
		int id = m->playlistDatabaseWrapper->getPlaylistByName(name).id();
		if(id >= 0){
			this->setId(id);
		}

		this->setTemporary(false);
		this->setName(name);
		this->setChanged(false);

		return SaveAsAnswer::Success;
	}

	return SaveAsAnswer::OtherError;
}

SaveAsAnswer DBInterface::rename(const QString& name)
{
	if(name.isEmpty()) {
		return Util::SaveAsAnswer::InvalidName;
	}

	CustomPlaylistSkeletons skeletons;
	m->playlistDatabaseWrapper->getAllSkeletons(skeletons);

	// check if name already exists
	bool exists = Util::Algorithm::contains(skeletons, [&name](auto skeleton){
		return (name.compare(skeleton.name(), Qt::CaseInsensitive) == 0);
	});

	if(exists){
		return SaveAsAnswer::NameAlreadyThere;
	}

	bool success = m->playlistDatabaseWrapper->renamePlaylist(m->id, name);
	if(success){
		this->setName(name);
		return SaveAsAnswer::Success;
	}

	return SaveAsAnswer::OtherError;
}

bool DBInterface::deletePlaylist()
{
	return m->playlistDatabaseWrapper->deletePlaylist(m->id);
}

bool DBInterface::removeFromDatabase()
{
	bool success;
	if(m->id >= 0){
		success = m->playlistDatabaseWrapper->deletePlaylist(m->id);
	}

	else{
		success = m->playlistDatabaseWrapper->deletePlaylist(m->name);
	}

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
