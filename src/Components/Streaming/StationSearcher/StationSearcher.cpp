/* StationSearcher.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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



#include "StationSearcher.h"
#include "FMStreamParser.h"
#include "Utils/WebAccess/AsyncWebAccess.h"

#include <QUrl>

struct StationSearcher::Private
{
	QList<RadioStation> found_stations;
	QString				search_string;
	int					current_page;
	StationSearcher::Mode mode;

	Private() :
		current_page(0),
		mode(StationSearcher::NewSearch)
	{}

	QString url()
	{
		if(mode == StationSearcher::Style)
		{
			return QString("http://fmstream.org/index.php?style=%1")
					.arg(search_string);
		}

		else if(current_page == 0)
		{
			return QString("http://fmstream.org/index.php?s=%1&cm=0")
					.arg(search_string);
		}

		else
		{
			return QString("http://fmstream.org/index.php?s=%1&n=%2")
					.arg(search_string)
					.arg(current_page);
		}
	}


	void increase_page()
	{
		current_page += 200;
	}

	void decrease_page()
	{
		current_page = std::max(0, current_page - 200);
	}
};


StationSearcher::StationSearcher(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

StationSearcher::~StationSearcher() {}



void StationSearcher::start_call()
{
	AsyncWebAccess* wa = new AsyncWebAccess(this);

	connect(wa, &AsyncWebAccess::sig_finished, this, &StationSearcher::search_finished);

	wa->run(m->url());
}

void StationSearcher::search_style(const QString& style)
{
	m->mode = StationSearcher::Style;
	m->search_string = style;

	start_call();
}


void StationSearcher::search_station(const QString& name)
{
	m->mode = StationSearcher::NewSearch;
	m->current_page = 0;
	m->search_string = name;

	start_call();
}

void StationSearcher::search_previous()
{
	m->decrease_page();
	m->mode = StationSearcher::Incremental;

	start_call();
}

void StationSearcher::search_next()
{
	m->increase_page();
	m->mode = StationSearcher::Incremental;

	start_call();
}

bool StationSearcher::can_search_next() const
{
	return (m->found_stations.size() > 10 && m->mode != StationSearcher::Style);
}

bool StationSearcher::can_search_previous() const
{
	return (m->current_page >= 200 && m->found_stations.size() > 0 && m->mode != StationSearcher::Style);
}

StationSearcher::Mode StationSearcher::mode() const
{
	return m->mode;
}

QList<RadioStation> StationSearcher::found_stations() const
{
	return m->found_stations;
}

void StationSearcher::search_finished()
{
	auto* wa = static_cast<AsyncWebAccess*>(sender());

	FMStreamParser parser(wa->data());
	m->found_stations = parser.stations();

	wa->deleteLater();

	emit sig_stations_found();
}
