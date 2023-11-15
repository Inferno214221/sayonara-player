/* AbstractStreamHandler.h */

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

#ifndef AbstractStreamHandler_H
#define AbstractStreamHandler_H

#include "Utils/Pimpl.h"
#include "Utils/Streams/Station.h"

#include <QObject>
#include <QList>

class PlaylistCreator;

class AbstractStationHandler :
	public QObject
{
	Q_OBJECT
	PIMPL(AbstractStationHandler)

	signals:
		void sigStopped();
		void sigError();
		void sigDataAvailable();
		void sigUrlCountExceeded(int urlCount, int maxUrlCount);

	public:
		explicit AbstractStationHandler(PlaylistCreator* playlistCreator, QObject* parent = nullptr);
		~AbstractStationHandler() override;

		bool parseStation(const StationPtr& station);

		virtual bool getAllStreams(QList<StationPtr>& streams) = 0;
		virtual bool addNewStream(const StationPtr& station) = 0;
		virtual bool deleteStream(const QString& name) = 0;
		virtual bool updateStream(const QString& name, const StationPtr& station) = 0;

		[[nodiscard]] virtual StationPtr station(const QString& name) = 0;

		void stop();

	protected:
		virtual MetaDataList preprocessPlaylist(const StationPtr& station, MetaDataList tracks) = 0;

	private:
		void createPlaylist(const StationPtr& station, const MetaDataList& tracks);

	private slots: // NOLINT(readability-redundant-access-specifiers)
		void parserFinished(bool success);
		void parserStopped();
};

#endif // AbstractStreamHandler_H
