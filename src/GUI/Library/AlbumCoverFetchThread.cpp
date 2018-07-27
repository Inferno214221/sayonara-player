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
#include "Utils/Utils.h"

#include <QList>
#include <QPair>

#include <atomic>
#include <mutex>

using Cover::Location;
using Cover::Lookup;
using Hash=AlbumCoverFetchThread::Hash;

struct AlbumCoverFetchThread::Private
{
	AlbumCoverFetchThread::HashAlbumList	hash_album_list;
	AlbumCoverFetchThread::HashLocationList	hash_location_list;

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
		hash_album_list.clear();
		hash_location_list.clear();
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
	const int PauseBetweenRequests = 10;
	const int NumParallelRequests = 10;

	while(m->may_run)
	{
		while(m->hash_album_list.isEmpty() || !m->goon)
		{
			Util::sleep_ms(300);
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
			if(m->hash_album_list.isEmpty()) {
				break;
			}

			if(i > 0) {
				Util::sleep_ms(PauseBetweenRequests);
			}

			std::lock_guard<std::mutex> guard(m->mutex);
			Q_UNUSED(guard)

			HashAlbumPair hap = m->hash_album_list.takeFirst();
			Cover::Location cl = Cover::Location::cover_location(hap.second);

			m->hash_location_list.push_back(HashLocationPair(hap.first, cl));

			if(m->may_run)
			{
				emit sig_next();
			}

			else
			{
				return;
			}
		}
	}
}

void AlbumCoverFetchThread::add_album(const Album& album)
{
	std::lock_guard<std::mutex> g1(m->mutex_add_data);
	Q_UNUSED(g1)

	QString hash = get_hash(album);
	bool has_hash = ::Util::contains(m->hash_album_list, [hash](const HashAlbumPair& p){
		return (p.first == hash);
	});

	if(has_hash){
		return;
	}

	has_hash = ::Util::contains(m->hash_location_list, [hash](const HashLocationPair& p){
		return (p.first == hash);
	});

	if(has_hash){
		return;
	}

	std::lock_guard<std::mutex> g2(m->mutex);
	Q_UNUSED(g2)

	m->hash_album_list.push_back(HashAlbumPair(hash, album));
}

AlbumCoverFetchThread::HashLocationPair AlbumCoverFetchThread::take_current_location()
{
	return m->hash_location_list.takeFirst();
}


AlbumCoverFetchThread::Hash AlbumCoverFetchThread::get_hash(const Album& album)
{
	return album.name() + "-" + QString::number(album.id);
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

