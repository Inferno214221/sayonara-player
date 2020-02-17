/* SomaFMPlaylistModel.cpp */

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

/* SomaFMPlaylistModel.cpp */

#include "SomaFMPlaylistModel.h"
#include "Components/Streaming/SomaFM/SomaFMStation.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverFetchManager.h"

#include "Utils/globals.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Gui/Utils/MimeDataUtils.h"
#include "Gui/Utils/CustomMimeData.h"

#include <QUrl>

using namespace Gui;
using SomaFM::PlaylistModel;

struct SomaFM::PlaylistModel::Private
{
	SomaFM::Station station;
};

SomaFM::PlaylistModel::PlaylistModel(QObject* parent) :
	QStringListModel(parent)
{
	m = Pimpl::make<Private>();
}

SomaFM::PlaylistModel::~PlaylistModel() = default;

void SomaFM::PlaylistModel::setStation(const SomaFM::Station& station)
{
	const QStringList urls = station.playlists();

	QStringList entries;
	for(const QString& url : urls)
	{
		SomaFM::Station::UrlType type = station.urlType(url);
		if(type == SomaFM::Station::UrlType::MP3){
			entries << station.name() + " (mp3)";
		}

		else if(type == SomaFM::Station::UrlType::AAC){
			entries << station.name() + " (aac)";
		}

		else {
			entries << url;
		}
	}

	this->setStringList(entries);
}

QMimeData* SomaFM::PlaylistModel::mimeData(const QModelIndexList& indexes) const
{
	if(indexes.isEmpty()){
		return nullptr;
	}

	int row = indexes.first().row();

	QStringList urls = m->station.playlists();
	if(!Util::between(row, urls)){
		return nullptr;
	}

	QString playlist_url = urls[row];

	auto* mime_data = new CustomMimeData(this);
	mime_data->setCoverUrl(":/soma_icons/soma.png");
	mime_data->setUrls({QUrl(playlist_url)});

	return mime_data;
}
