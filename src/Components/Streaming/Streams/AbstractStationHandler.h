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

/**
 * @brief Used to interprete website data as streams. Some methods have to be overridden,
 * to map their functions to their specific database functions.
 * The track list is held in a map, which is accessible through its station name. It can be
 * accessed via the get_tracks() method.
 * @ingroup Streams
 */
class AbstractStationHandler :
	public QObject
{
	Q_OBJECT
	PIMPL(AbstractStationHandler)

	public:
		explicit AbstractStationHandler(PlaylistCreator* playlistCreator, QObject* parent = nullptr);
		virtual ~AbstractStationHandler();

	signals:
		void sigStopped();
		void sigError();
		void sigDataAvailable();
		void sigUrlCountExceeded(int urlCount, int maxUrlCount);

	public:
		/**
		 * @brief Retrieves data from the station and tries to interprete it via the parse_content() method.
		 * @param url url to retrieve the data from
		 * @param station_name the station name
		 * @return true, if no other station is parsed atm, false else
		 */
		bool parseStation(StationPtr station);

		/**
		 * @brief Saves the station. Calls the add_stream() method.
		 * @param station_name The station name.
		 * @param url the station url.
		 */
		bool save(StationPtr station);

		/**
		 * @brief This method should return all stations in database
		 * @param streams target StreamMap
		 * @return true if successful, false else
		 */
		virtual bool getAllStreams(QList<StationPtr>& streams) = 0;

		/**
		 * @brief This method should add a new station to database. If the station
		 * already exists, there should be a corresponding error handling.
		 * @param station_name station name
		 * @param url url
		 * @return true if successful, false else
		 */
		virtual bool addNewStream(StationPtr station) = 0;

		virtual StationPtr createStreamInstance(const QString& name, const QString& url) const = 0;

		/**
		 * @brief Delete a station from the database.
		 * @param station_name the station to be deleted
		 * @return true if successful, false else
		 */
		virtual bool deleteStream(const QString& name) = 0;

		/**
		 * @brief Update the url of a station
		 * @param station_name the station to be updated
		 * @param url the new url
		 * @return true if successful, false else
		 */
		virtual bool update(const QString& name, StationPtr station) = 0;

		virtual StationPtr station(const QString& name) = 0;

		/**
		 * @brief Clears all station content
		 */
		void stop();

	protected:
		virtual MetaDataList preprocessPlaylist(StationPtr station, MetaDataList tracks) = 0;

	private:
		void createPlaylist(StationPtr station, const MetaDataList& tracks);

	private slots: // NOLINT(readability-redundant-access-specifiers)
		void parserFinished(bool success);
		void parserStopped();
};

#endif // AbstractStreamHandler_H
