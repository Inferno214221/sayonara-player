/* SomaFMPlaylistModel.cpp */

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

SomaFM::PlaylistModel::~PlaylistModel() {}

void SomaFM::PlaylistModel::set_station(const SomaFM::Station& station)
{
	QStringList urls = station.urls();
	QStringList entries;

	for(QString& url : urls)
	{
		SomaFM::Station::UrlType type = station.url_type(url);
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

	QStringList urls = m->station.urls();
	if(!Util::between(row, urls)){
		return nullptr;
	}

	QUrl url( urls[row] );

	CustomMimeData* mime_data = new CustomMimeData(this);
	Cover::Location location = m->station.cover_location();

	MetaDataList v_md;

	for(const QString& url : urls)
	{
		MetaData md;
			md.set_radio_station(url);
			md.set_title(m->station.name());
			md.set_filepath(url);

		QStringList cover_urls;
		auto fetcher_urls = location.search_urls(false);
		for(auto fetcher_url : fetcher_urls)
		{
			cover_urls << fetcher_url.url;
		}

		md.set_cover_download_urls(cover_urls);

		v_md << md;
	}

	mime_data->set_metadata(v_md);
	mime_data->setUrls({url});
	if(location.has_search_urls())
	{
		auto search_urls = location.search_urls(false);
		if(!search_urls.isEmpty())
		{
			mime_data->set_cover_url(search_urls.first().url);
		}
	}

	return mime_data;
}
