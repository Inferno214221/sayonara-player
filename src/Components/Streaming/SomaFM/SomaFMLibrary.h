/* SomaFMLibrary.h */

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

/* SomaFMLibrary.h */

#ifndef SOMAFMLIBRARY_H
#define SOMAFMLIBRARY_H

#include <QObject>

#include "Utils/Pimpl.h"


namespace SomaFM
{
	class Station;
	class Library : public QObject
	{
		Q_OBJECT
		PIMPL(Library)

		signals:
			void sigStationsLoaded(const QList<SomaFM::Station>& stations);
			void sigStationChanged(const SomaFM::Station& station);
			void sigLoadingFinished();
			void sigLoadingStarted();

		public:
			explicit Library(QObject* parent=nullptr);
			~Library();

			Station station(const QString& name);
			void createPlaylistFromStation(int idx);
			bool createPlaylistFromStreamlist(int idx);
			void searchStations();
			void setStationLoved(const QString& stationName, bool loved);

		public:
			static void parseMetadataForPlaylist(MetaDataList& tracks, const SomaFM::Station& station);

		private slots:
			void websiteFetched();
			void playlistContentFetched(bool success);
			void stationStreamsFetched(bool success);
	};
}

#endif // SOMAFMLIBRARY_H
