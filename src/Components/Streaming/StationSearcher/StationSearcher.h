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

class StationParser
{
	public:
		virtual ~StationParser();

		[[nodiscard]] virtual QList<RadioStation> parse(const QByteArray& data) const = 0;
};

class StationSearcher :
	public QObject
{
	Q_OBJECT
	PIMPL(StationSearcher)

	signals:
		void sigStationsFound();

	public:
		enum Mode
		{
			ByName,
			ByStyle
		};

		explicit StationSearcher(QObject* parent);
		~StationSearcher() override;

		void searchStyle(const QString& style);
		void searchStation(const QString& name);
		void searchPrevious();
		void searchNext();

		[[nodiscard]] virtual bool canSearchNext() const;
		[[nodiscard]] virtual bool canSearchPrevious() const;
		[[nodiscard]] Mode mode() const;
		[[nodiscard]] const QList<RadioStation>& foundStations() const;

	protected:
		[[nodiscard]] virtual QString
		buildUrl(const QString& searchtext, Mode mode, int page, int maxEntries) const = 0;

		[[nodiscard]] virtual std::unique_ptr<StationParser> createStationParser() = 0;

	private slots:
		void startCall();
		void searchFinished();
};

#endif // STATIONSEARCHER_H
