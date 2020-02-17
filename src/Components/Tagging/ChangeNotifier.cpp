/* MetaDataChangeNotifier.cpp */

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

#include "ChangeNotifier.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"

using namespace Tagging;

struct ChangeNotifier::Private
{
	MetaDataList oldTracks;
	MetaDataList newTracks;
	MetaDataList deletedTracks;

	AlbumList oldAlbums;
	AlbumList newAlbums;
};

ChangeNotifier::ChangeNotifier(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

ChangeNotifier::~ChangeNotifier() = default;

void ChangeNotifier::changeMetadata(const MetaDataList& oldTracks, const MetaDataList& newTracks)
{
	m->oldTracks = oldTracks;
	m->newTracks = newTracks;

	emit sigMetadataChanged();
}

void ChangeNotifier::deleteMetadata(const MetaDataList& deletedTracks)
{
	m->deletedTracks = deletedTracks;

	emit sigMetadataDeleted();
}

void ChangeNotifier::updateAlbums(const AlbumList& oldAlbums, const AlbumList& newAlbums)
{
	m->oldAlbums = oldAlbums;
	m->newAlbums = newAlbums;

	emit sigAlbumsChanged();
}

QPair<MetaDataList, MetaDataList> ChangeNotifier::changedMetadata() const
{
	QPair<MetaDataList, MetaDataList> ret
	{
		m->oldTracks,
		m->newTracks
	};

	return ret;
}

MetaDataList ChangeNotifier::deletedMetadata() const
{
	return m->deletedTracks;
}

QPair<AlbumList, AlbumList> ChangeNotifier::changedAlbums() const
{
	QPair<AlbumList, AlbumList> ret
	{
		m->oldAlbums,
		m->newAlbums
	};

	return ret;
}
