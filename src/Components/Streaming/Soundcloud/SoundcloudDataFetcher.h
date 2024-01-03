/* SoundcloudDataFetcher.h */

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

#ifndef SOUNDCLOUDDATAFETCHER_H
#define SOUNDCLOUDDATAFETCHER_H

#include <QObject>
#include "Utils/Pimpl.h"

class ArtistList;
class AlbumList;
class MetaDataList;

namespace SC
{
	class DataFetcher :
		public QObject
	{
		Q_OBJECT
		PIMPL(DataFetcher)

		signals:
			void sigExtArtistsFetched(const ArtistList& artists);
			void sigArtistsFetched(const ArtistList& artists);
			void sigPlaylistsFetched(const AlbumList& albums);
			void sigTracksFetched(const MetaDataList& tracks);

		public:
			explicit DataFetcher(QObject* parent = nullptr);
			~DataFetcher() override;

			void searchArtists(const QString& artistName);
			void getArtist(int artistId);
			void getTracksByArtist(int artistId);

		private slots:
			void artistsFetched();
			void playlistsFetched();
			void tracksFetched();
	};
}

#endif // SOUNDCLOUDDataFetcher_H
