/* SoundcloudDataFetcher.cpp */

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

#include "SoundcloudDataFetcher.h"
#include "SoundcloudWebAccess.h"
#include "SoundcloudJsonParser.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/WebAccess/WebClientImpl.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Settings/Settings.h"

#include <QMap>

namespace
{
	QString oauthToken()
	{
		return GetSetting(SetNoDB::Soundcloud_AuthToken);
	}

	QMap<QByteArray, QByteArray> requestHeader()
	{
		return QMap<QByteArray, QByteArray> {
			{"accept",        "application/json; charset=utf-8"},
			{"Authorization", QString("OAuth %1").arg(oauthToken()).toLocal8Bit()}
		};
	}
}

namespace SC
{
	struct DataFetcher::Private
	{
		MetaDataList tracks;
		AlbumList playlists;
		ArtistList artists;
		int artistId {-1};
	};

	DataFetcher::DataFetcher(QObject* parent) :
		QObject(parent)
	{
		m = Pimpl::make<DataFetcher::Private>();
	}

	DataFetcher::~DataFetcher() = default;

	void DataFetcher::searchArtists(const QString& artistName)
	{
		m->artistId = -1;
		m->artists.clear();
		m->playlists.clear();
		m->tracks.clear();

		auto* webClient = new WebClientImpl(this);
		connect(webClient, &WebClient::sigFinished, this, &DataFetcher::artistsFetched);

		webClient->setRawHeader(requestHeader());
		webClient->run(createLinkGetArtist(artistName));
	}

	void DataFetcher::getArtist(int artistId)
	{
		m->artistId = -1;
		m->artists.clear();
		m->playlists.clear();
		m->tracks.clear();

		auto* webClient = new WebClientImpl(this);
		connect(webClient, &WebClient::sigFinished, this, &DataFetcher::artistsFetched);

		webClient->setRawHeader(requestHeader());
		webClient->run(createLinkGetArtist(artistId));
	}

	void DataFetcher::artistsFetched()
	{
		auto* webClient = dynamic_cast<WebClient*>(sender());
		if(webClient->status() == WebClient::Status::GotData)
		{
			const auto parser = JsonParser(webClient->data());
			parser.parseArtists(m->artists);

			emit sigArtistsFetched(m->artists);
		}

		webClient->deleteLater();
	}

	void DataFetcher::getTracksByArtist(int artistId)
	{
		m->artistId = artistId;
		m->artists.clear();
		m->playlists.clear();
		m->tracks.clear();

		auto* webClient = new WebClientImpl(this);
		connect(webClient, &WebClient::sigFinished, this, &DataFetcher::playlistsFetched);

		webClient->setRawHeader(requestHeader());
		webClient->run(createLinkGetPlaylists(artistId));
	}

	void DataFetcher::playlistsFetched()
	{
		auto* webClient = dynamic_cast<WebClient*>(sender());
		if(webClient->status() == WebClient::Status::GotData)
		{
			const auto parser = JsonParser(webClient->data());
			parser.parsePlaylists(m->artists, m->playlists, m->tracks);

			auto emptyAlbum = Album{};
			emptyAlbum.setId(0);
			m->playlists.appendUnique(AlbumList() << emptyAlbum);

			auto* awaTracks = new WebClientImpl(this);
			connect(awaTracks, &WebClient::sigFinished, this, &DataFetcher::tracksFetched);

			awaTracks->setRawHeader(requestHeader());
			awaTracks->run(createLinkGetTracks(m->artistId));
		}

		webClient->deleteLater();
	}

	void DataFetcher::tracksFetched()
	{
		auto* webClient = dynamic_cast<WebClient*>(sender());
		if(webClient->status() == WebClient::Status::GotData)
		{
			auto tracks = MetaDataList {};
			auto artists = ArtistList {};

			const auto parser = JsonParser(webClient->data());
			parser.parseTracks(artists, tracks);

			m->tracks.appendUnique(tracks);
			m->artists.appendUnique(artists);

			emit sigPlaylistsFetched(m->playlists);
			emit sigTracksFetched(m->tracks);
			emit sigExtArtistsFetched(m->artists);
		}

		webClient->deleteLater();
	}
}