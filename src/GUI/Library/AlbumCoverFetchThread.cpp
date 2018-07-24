/* AlbumCoverFetchThread.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#include "AlbumCoverFetchThread.h"
#include "Utils/MetaData/Album.h"
#include "Components/Covers/CoverLookup.h"
#include "Components/Covers/CoverLocation.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include <atomic>
#include <mutex>

using Cover::Location;
using Cover::Lookup;

struct AlbumCoverFetchThread::Private
{
	QString				current_hash;
	Location			current_cl;
	QStringList			hashes;
	QList<Location>		cover_locations;

	std::atomic<bool>	goon;
	std::mutex			mutex, mutex_add_data;

	bool				may_run;

	Private()
	{
		init();
	}

	void init()
	{
		may_run = true;
		goon = true;
		hashes.clear();
		cover_locations.clear();
		current_hash = QString();
		current_cl = Location();
	}
};


AlbumCoverFetchThread::AlbumCoverFetchThread(QObject* parent) :
	QThread(parent)
{
	m = Pimpl::make<Private>();
}

AlbumCoverFetchThread::~AlbumCoverFetchThread()
{
	m->may_run = false;

	while(this->isRunning()){
		Util::sleep_ms(50);
	}
}


void AlbumCoverFetchThread::run()
{
	m->init();
	const int PauseBetweenRequests = 300;
	const int NumParallelRequests = 10;

	while(m->may_run)
	{
		while(m->hashes.isEmpty() || !m->goon)
		{
			Util::sleep_ms(PauseBetweenRequests);
			if(!m->may_run){
				return;
			}
		}

		m->goon = false;

		if(!m->may_run){
			return;
		}

		for(int i=0; i<NumParallelRequests; i++)
		{
			if(m->hashes.isEmpty()) {
				break;
			}

			if(i > 0) {
				Util::sleep_ms(PauseBetweenRequests);
			}

			try{
				std::lock_guard<std::mutex> guard(m->mutex);
				m->current_hash = m->hashes.takeFirst();
				m->current_cl = m->cover_locations.takeFirst();

				emit sig_next();

			} catch(std::exception* e) {
				sp_log(Log::Warning, this) << "1 Exception" << e->what();
				Util::sleep_ms(PauseBetweenRequests);
			}

			if(!m->may_run){
				return;
			}
		}
	}
}


void AlbumCoverFetchThread::add_data(const QString& hash, const Location& cl)
{
	std::lock_guard<std::mutex> guard(m->mutex_add_data);
	Q_UNUSED(guard)

	if(m->hashes.contains(hash) || m->current_hash.compare(hash) == 0){
		return;
	}

	bool done = false;
	while(!done)
	{
		try
		{
			std::lock_guard<std::mutex> guard(m->mutex);
			Q_UNUSED(guard)

			m->hashes.push_front(hash);
			m->cover_locations.push_front(cl);
			done = true;

		}

		catch(std::exception* e)
		{
			sp_log(Log::Warning, this) << "2 Exception" << e->what();
			Util::sleep_ms(10);
		}
	}
}


QString AlbumCoverFetchThread::current_hash() const
{
	return m->current_hash;
}

Location AlbumCoverFetchThread::current_cover_location() const
{
	return m->current_cl;
}


void AlbumCoverFetchThread::done(bool success)
{
	Q_UNUSED(success)

	if(m){
		m->goon = true;
	}
}

void AlbumCoverFetchThread::stop()
{
	if(m){
		m->may_run = false;
	}
}

