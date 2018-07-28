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
#include <QFile>

#include <atomic>
#include <mutex>

using Cover::Location;
using Cover::Lookup;
using Hash=AlbumCoverFetchThread::Hash;

#define LOCK_GUARD(locking_mutex) std::lock_guard<std::mutex> g(locking_mutex); Q_UNUSED(g)

struct AlbumCoverFetchThread::Private
{
	AlbumCoverFetchThread::HashAlbumList	hash_album_list;
	AlbumCoverFetchThread::HashLocationList	hash_location_list;
	AlbumCoverFetchThread::HashLocationList	hash_location_online_list;

	std::mutex			mutex_album_list, mutex_location_list, mutex_goon;

	std::atomic<int>	goon;
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
	sp_log(Log::Debug, this) << "Destructor called";
	m->may_run = false;
}


void AlbumCoverFetchThread::run()
{
	m->init();

	while(true)
	{
		while(!m->may_run)
		{
			::Util::sleep_ms(100);
		}

		int c = m->hash_album_list.count();
		while(c == 0)
		{
			Util::sleep_ms(100);
			if(!m->may_run)
			{
				break;
			}

			c = m->hash_album_list.count();
		}

		for(int i=0; i<c; i++)
		{
			if(m->hash_album_list.isEmpty()) {
				break;
			}

			HashAlbumPair hap;
			{
				LOCK_GUARD(m->mutex_album_list)
				if(m->hash_album_list.isEmpty()){
					continue;
				}

				hap = m->hash_album_list.takeLast();
			}

			if(!m->may_run)
			{
				break;
			}

			Cover::Location cl = Cover::Location::cover_location(hap.second);
			{
				LOCK_GUARD(m->mutex_location_list)
				if(cl.has_audio_file_source() ||
					( !Location::is_invalid(cl.preferred_path()) &&
					  QFile::exists(cl.preferred_path())
					)
				  )
				{
					m->hash_location_list.push_back(HashLocationPair(hap.first, cl));
				}

				else
				{
					m->hash_location_online_list.push_back(HashLocationPair(hap.first, cl));
				}
			}

			if(!m->may_run)
			{
				break;
			}

			emit sig_next();
		}
	}
}

void AlbumCoverFetchThread::add_album(const Album& album)
{
	bool has_hash;
	QString hash = get_hash(album);

	{
		LOCK_GUARD(m->mutex_album_list);
		has_hash = ::Util::contains(m->hash_album_list, [hash](const HashAlbumPair& p){
			return (p.first == hash);
		});
	}

	if(has_hash){
		return;
	}

	{
		LOCK_GUARD(m->mutex_location_list)
		has_hash = ::Util::contains(m->hash_location_list, [hash](const HashLocationPair& p){
			return (p.first == hash);
		});
	}

	if(has_hash){
		return;
	}

	{
		LOCK_GUARD(m->mutex_album_list);
		m->hash_album_list.push_back(HashAlbumPair(hash, album));
	}
}

AlbumCoverFetchThread::HashLocationPair AlbumCoverFetchThread::take_current_location()
{
	LOCK_GUARD(m->mutex_location_list);
	if(m->hash_location_list.count() > 0){
		return m->hash_location_list.takeLast();
	}

	else if(m->hash_location_online_list.count() > 0){
		sp_log(Log::Warning, this) << "Search online cover";
		return m->hash_location_online_list.takeLast();
	}

	else {
		return AlbumCoverFetchThread::HashLocationPair();
	}
}


AlbumCoverFetchThread::Hash AlbumCoverFetchThread::get_hash(const Album& album)
{
	return album.name() + "-" + QString::number(album.id);
}


void AlbumCoverFetchThread::done(bool success)
{
	Q_UNUSED(success)

	LOCK_GUARD(m->mutex_goon);
	m->goon++;

}

void AlbumCoverFetchThread::pause()
{
	m->may_run = false;

	{
		LOCK_GUARD(m->mutex_album_list)
		m->hash_album_list.clear();
	}

	{
		LOCK_GUARD(m->mutex_location_list)
		m->hash_location_list.clear();
		m->hash_location_online_list.clear();
	}
}

void AlbumCoverFetchThread::resume()
{
	::Util::sleep_ms(100);
	m->may_run = true;
}


