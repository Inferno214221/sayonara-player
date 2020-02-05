/* StationSearcher.h */

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

#ifndef STATIONSEARCHER_H
#define STATIONSEARCHER_H

#include <QObject>
#include "Utils/Pimpl.h"
#include "RadioStation.h"

class StationSearcher : public QObject
{
	Q_OBJECT
	PIMPL(StationSearcher)

signals:
	void sig_stations_found();

private:
	void start_call();

public:

	enum Mode
	{
		NewSearch,
		Incremental,
		Style
	};

	StationSearcher(QObject* parent=nullptr);
	~StationSearcher();

	void search_style(const QString& style);
	void search_station(const QString& name);
	void search_previous();
	void search_next();

	bool can_search_next() const;
	bool can_search_previous() const;
	Mode mode() const;

	QList<RadioStation> found_stations() const;

private slots:
	void search_finished();
};

#endif // STATIONSEARCHER_H
