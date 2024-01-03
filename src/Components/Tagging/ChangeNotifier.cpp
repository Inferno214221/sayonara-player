/* MetaDataChangeNotifier.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
	QList<MetaDataPair> changedTracks;
	MetaDataList deletedTracks;
	QList<AlbumPair> changedAlbums;
};

ChangeNotifier::ChangeNotifier(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

ChangeNotifier::~ChangeNotifier() = default;

void ChangeNotifier::clearChangedMetadata()
{
	m->changedTracks.clear();
	emit sigMetadataChanged();
}

void ChangeNotifier::changeMetadata(const QList<MetaDataPair>& changedTracks)
{
	m->changedTracks = changedTracks;
	emit sigMetadataChanged();
}

void ChangeNotifier::deleteMetadata(const MetaDataList& deletedTracks)
{
	m->deletedTracks = deletedTracks;
	emit sigMetadataDeleted();
}

void ChangeNotifier::updateAlbums(const QList<AlbumPair>& changedAlbums)
{
	m->changedAlbums = changedAlbums;
	emit sigAlbumsChanged();
}

const QList<MetaDataPair>& ChangeNotifier::changedMetadata() const
{
	return m->changedTracks;
}

const MetaDataList& ChangeNotifier::deletedMetadata() const
{
	return m->deletedTracks;
}

const QList<AlbumPair>& ChangeNotifier::changedAlbums() const
{
	return m->changedAlbums;
}
