/* StreamHandlerStreams.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef STREAMHANDLERSTREAMS_H
#define STREAMHANDLERSTREAMS_H

#include "AbstractStationHandler.h"

class StreamHandler :
	public AbstractStationHandler
{
public:
	explicit StreamHandler(QObject* parent=nullptr);
	~StreamHandler() override;

	bool get_all_streams(QList<StationPtr>& stations) override;
	bool add_stream(StationPtr station) override;
	bool delete_stream(const QString& station_name) override;
	bool update_url(const QString& station_name, const QString& url) override;
	bool rename(const QString& old_name, const QString& new_name) override;
	StationPtr create_stream(const QString& name, const QString& url) const override;
	StationPtr station(const QString &name) override;
};


#endif // STREAMHANDLERSTREAMS_H
