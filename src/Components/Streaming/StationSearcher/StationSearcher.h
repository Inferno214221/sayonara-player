/* StationSearcher.h */

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

#ifndef STATIONSEARCHER_H
#define STATIONSEARCHER_H

#include "Utils/Pimpl.h"

#include <QObject>

struct RadioStation;
class StationSearcher :
	public QObject
{
	Q_OBJECT
	PIMPL(StationSearcher)

	signals:
		void sigStationsFound();

	private:
		void startCall();

	public:
		enum Mode
		{
			NewSearch,
			Incremental,
			Style
		};

		StationSearcher(QObject* parent = nullptr);
		~StationSearcher();

		void searchStyle(const QString& style);
		void searchStation(const QString& name);
		void searchPrevious();
		void searchNext();

		bool canSearchNext() const;
		bool canSearchPrevious() const;
		Mode mode() const;

		const QList<RadioStation>& foundStations() const;

	private slots:
		void searchFinished();
};

#endif // STATIONSEARCHER_H
