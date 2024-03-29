/* StreamHandlerPodcasts.h */

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

#ifndef STREAMHANDLERPODCASTS_H
#define STREAMHANDLERPODCASTS_H

#include "AbstractStationHandler.h"

namespace Playlist
{
	class Creator;
}

class PodcastHandler :
	public AbstractStationHandler
{
	Q_OBJECT
	public:
		PodcastHandler(Playlist::Creator* playlistCreator,
		               const std::shared_ptr<StationParserFactory>& stationParserFactory,
		               QObject* parent = nullptr);
		~PodcastHandler() override;

		bool getAllStreams(QList<StationPtr>& stations) override;
		bool updateStream(const QString& stationName, const StationPtr& station) override;

	protected:
		MetaDataList preprocessPlaylist(const StationPtr& station, MetaDataList tracks) override;
		[[nodiscard]] StationPtr fetchStation(const QString& name) override;
		bool saveStream(const StationPtr& station) override;
		bool deleteStream(const QString& stationName) override;
};

#endif // STREAMHANDLERPODCASTS_H
