/* MetaDataChangeNotifier.cpp */

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

#include "ChangeNotifier.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"

using namespace Tagging;

struct ChangeNotifier::Private
{
	MetaDataList v_md_old;
	MetaDataList v_md_new;
	MetaDataList v_md_deleted;

	AlbumList albums_old;
	AlbumList albums_new;
};

ChangeNotifier::ChangeNotifier(QObject *parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

ChangeNotifier::~ChangeNotifier() = default;

void ChangeNotifier::change_metadata(const MetaDataList& v_md_old, const MetaDataList& v_md_new)
{
	m->v_md_old = v_md_old;
	m->v_md_new = v_md_new;

	emit sig_metadata_changed();
}

void ChangeNotifier::delete_metadata(const MetaDataList& v_md_deleted)
{
	m->v_md_deleted = v_md_deleted;

	emit sig_metadata_deleted();
}

void ChangeNotifier::update_albums(const AlbumList& albums_old, const AlbumList& albums_new)
{
	m->albums_old = albums_old;
	m->albums_new = albums_new;

	emit sig_albums_changed();
}

QPair<MetaDataList, MetaDataList> ChangeNotifier::changed_metadata() const
{
	QPair<MetaDataList, MetaDataList> ret
	{
		m->v_md_old,
		m->v_md_new
	};

	return ret;
}

MetaDataList ChangeNotifier::deleted_metadata() const
{
	return m->v_md_deleted;
}

QPair<AlbumList, AlbumList> ChangeNotifier::changed_albums() const
{
	QPair<AlbumList, AlbumList> ret
	{
		m->albums_old,
		m->albums_new
	};

	return ret;
}
