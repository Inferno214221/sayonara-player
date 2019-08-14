/* PlaylistDBInterface.cpp */

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
#include "PlaylistDBInterface.h"

#include "Utils/Algorithm.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Language/Language.h"

namespace Algorithm=Util::Algorithm;
using Playlist::DBInterface;
using Util::SaveAsAnswer;

struct DBInterface::Private
{
	Playlist::DBWrapper*  playlist_db_connector=nullptr;
	QString             name;
	bool                is_temporary;
	int                 id;

	Private(const QString& name) :
		name(name),
		is_temporary(true)
	{
		playlist_db_connector = new DBWrapper();
		id = playlist_db_connector->get_playlist_by_name(name).id();
	}

	~Private()
	{
		delete playlist_db_connector; playlist_db_connector=nullptr;
	}
};

DBInterface::DBInterface(const QString& name)
{
	m = Pimpl::make<Private>(name);
}

DBInterface::~DBInterface() {}

int DBInterface::get_id() const
{
	return m->id;
}

void DBInterface::set_id(int id)
{
	m->id = id;
}

QString DBInterface::get_name() const
{
	return m->name;
}

void DBInterface::set_name(const QString& name)
{
	m->name = name;
}

bool DBInterface::is_temporary() const
{
	return m->is_temporary;
}

void DBInterface::set_temporary(bool b)
{
	m->is_temporary = b;
}

SaveAsAnswer DBInterface::save()
{
	if(!is_storable()){
		return SaveAsAnswer::NotStorable;
	}

	if(m->id >= 0)
	{
		bool success = m->playlist_db_connector->save_playlist(tracks(), m->id, m->is_temporary);
		if(success) {
			this->set_changed(false);
			return SaveAsAnswer::Success;
		}

		else {
			return SaveAsAnswer::OtherError;
		}
	}

	else {
		return save_as(m->name, true);
	}
}

bool DBInterface::insert_temporary_into_db()
{
	if(!m->is_temporary || !is_storable()) {
		return false;
	}

	bool success = m->playlist_db_connector->save_playlist_temporary(tracks(), m->name);
	if(success) {
		m->id = m->playlist_db_connector->get_playlist_by_name(m->name).id();
	}

	return success;
}

SaveAsAnswer DBInterface::save_as(const QString& name, bool force_override)
{
	if(name.isEmpty()) {
		return Util::SaveAsAnswer::InvalidName;
	}

	if(!is_storable()){
		return Util::InvalidObject;
	}

	CustomPlaylistSkeletons skeletons;
	m->playlist_db_connector->get_all_skeletons(skeletons);

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
		success = m->playlist_db_connector->save_playlist(this->tracks(), tgt_id, m->is_temporary);
	}

	// New playlist
	else
	{
		success = m->playlist_db_connector->save_playlist_as(this->tracks(), name);

		if(success && this->is_temporary())
		{
			int old_id = this->get_id();
			m->playlist_db_connector->delete_playlist(old_id);
		}
	}

	if(success)
	{
		int id = m->playlist_db_connector->get_playlist_by_name(name).id();
		if(id >= 0){
			this->set_id(id);
		}

		this->set_temporary(false);
		this->set_name(name);
		this->set_changed(false);

		return SaveAsAnswer::Success;
	}

	return SaveAsAnswer::OtherError;
}

SaveAsAnswer DBInterface::rename(const QString& name)
{
	if(name.isEmpty()) {
		return Util::SaveAsAnswer::InvalidName;
	}

	if(!is_storable()){
		return SaveAsAnswer::NotStorable;
	}

	CustomPlaylistSkeletons skeletons;
	m->playlist_db_connector->get_all_skeletons(skeletons);

	// check if name already exists
	bool exists = Util::Algorithm::contains(skeletons, [&name](auto skeleton){
		return (name.compare(skeleton.name(), Qt::CaseInsensitive) == 0);
	});

	if(exists){
		return SaveAsAnswer::NameAlreadyThere;
	}

	bool success = m->playlist_db_connector->rename_playlist(m->id, name);
	if(success){
		this->set_name(name);
		return SaveAsAnswer::Success;
	}

	return SaveAsAnswer::OtherError;
}

bool DBInterface::delete_playlist()
{
	return m->playlist_db_connector->delete_playlist(m->id);
}

bool DBInterface::remove_from_db()
{
	if(!is_storable()){
		return false;
	}

	bool success;
	if(m->id >= 0){
		success = m->playlist_db_connector->delete_playlist(m->id);
	}

	else{
		success = m->playlist_db_connector->delete_playlist(m->name);
	}

	m->is_temporary = true;
	return success;
}

QString DBInterface::request_new_db_name(QString prefix)
{
	if(prefix.isEmpty())
	{
		prefix = Lang::get(Lang::New);
	}

	CustomPlaylistSkeletons skeletons;

	auto pdw = std::make_shared<DBWrapper>();
	pdw->get_all_skeletons(skeletons);

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
