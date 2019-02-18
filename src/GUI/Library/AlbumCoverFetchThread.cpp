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
#include "Utils/FileUtils.h"

#include <QFile>
#include <QFileInfo>

#include <atomic>
#include <mutex>

using Cover::Location;
using Cover::Lookup;
using Hash=AlbumCoverFetchThread::Hash;
using AtomicBool=std::atomic<bool>;
using AtomicInt=std::atomic<int>;

namespace FileUtils=::Util::File;

struct AlbumCoverFetchThread::Private
{
	AlbumCoverFetchThread::HashAlbumList hash_album_list;
	AlbumCoverFetchThread::HashLocationList	location_list;
	AlbumCoverFetchThread::HashLocationList	location_symlink_list;
	AlbumCoverFetchThread::HashLocationList	location_online_list;

	QStringList			queued_hashes;

	std::mutex mutex_album_list;
	std::mutex mutex_location_list;
	std::mutex mutex_symlink_list;
	std::mutex mutex_online_list;
	std::mutex mutex_queued_hashes;

	AtomicInt	done;
	AtomicBool	paused;
	AtomicBool	in_paused_state;
	AtomicBool	stopped;

	Private()
	{
		init();
	}

	void init()
	{
		done = 0;
		paused = false;
		in_paused_state = false;
		stopped = false;
		hash_album_list.clear();
		location_list.clear();
	}

	bool may_run()
	{
		return ((paused == false) && (stopped == false));
	}
};


AlbumCoverFetchThread::AlbumCoverFetchThread(QObject* parent) :
	QThread(parent)
{
	m = Pimpl::make<Private>();

	this->setObjectName("AlbumCoverFetchThread" + ::Util::random_string(4));
}

AlbumCoverFetchThread::~AlbumCoverFetchThread() {}


void AlbumCoverFetchThread::run()
{
	const int MaxThreads=10;
	m->init();

	while(!m->stopped)
	{
		while(m->paused)
		{
			m->in_paused_state = true;
			::Util::sleep_ms(10);

			if(m->stopped){
				return;
			}
		}

		m->in_paused_state = false;

		int c = m->hash_album_list.count();
		while(c == 0 && (m->may_run() == true))
		{
			Util::sleep_ms(10);

			c = m->hash_album_list.count();
		}

		for(int i=0; (m->may_run() == true) && (i<c); i++)
		{
			bool success = thread_create_cover_location();
			if(!success)
			{
				continue;
			}

			int qhc;
			{
				LOCK_GUARD(m->mutex_queued_hashes)
				qhc = m->queued_hashes.count();
			}

			while
			(
				(qhc > MaxThreads) &&
				(m->location_list.isEmpty()) &&
				(m->may_run())
			)
			{
				Util::sleep_ms(10);

				LOCK_GUARD(m->mutex_queued_hashes)
				qhc = m->queued_hashes.count();
			}

			if(m->may_run())
			{
				emit sig_next();
			}
		}
	}
}

bool AlbumCoverFetchThread::thread_create_cover_location()
{
	if(!m->may_run()){
		return false;
	}

	HashAlbumPair hap;
	{
		LOCK_GUARD(m->mutex_album_list)
		if(m->hash_album_list.isEmpty()){
			return false;
		}

		hap = m->hash_album_list.takeLast();
	}

	QString hash = hap.first;
	Album album = hap.second;

	Cover::Location cl = Cover::Location::cover_location(album);
	{
		QString cp = cl.cover_path();

		if( FileUtils::exists(cp) )
		{
			if(!QFileInfo(cp).isSymLink())
			{
				LOCK_GUARD(m->mutex_location_list)
				m->location_list.push_back(HashLocationPair(hash, cl));
			}

			else
			{
				LOCK_GUARD(m->mutex_symlink_list)
				m->location_symlink_list.push_back(HashLocationPair(hash, cl));
			}
		}

		else
		{
			LOCK_GUARD(m->mutex_online_list)
			m->location_online_list.push_back(HashLocationPair(hash, cl));
		}
	}

	return true;
}

void AlbumCoverFetchThread::add_album(const Album& album)
{
	if(m->stopped){
		return;
	}

	QString hash = get_hash(album);
	{
		LOCK_GUARD(m->mutex_album_list);
		bool has_hash = ::Util::contains(m->hash_album_list, [hash](const HashAlbumPair& p){
			return (p.first == hash);
		});

		if(has_hash){
			return;
		}
	}

	{
		LOCK_GUARD(m->mutex_location_list)
		bool has_hash = ::Util::contains(m->location_list, [hash](const HashLocationPair& p){
			return (p.first == hash);
		});

		if(has_hash){
			return;
		}
	}

	{
		LOCK_GUARD(m->mutex_online_list)
		bool has_hash = ::Util::contains(m->location_online_list, [hash](const HashLocationPair& p){
			return (p.first == hash);
		});

		if(has_hash){
			return;
		}
	}

	{
		LOCK_GUARD(m->mutex_symlink_list)
		bool has_hash = ::Util::contains(m->location_symlink_list, [hash](const HashLocationPair& p){
			return (p.first == hash);
		});

		if(has_hash){
			return;
		}
	}

	{
		LOCK_GUARD(m->mutex_queued_hashes)
		if(m->queued_hashes.contains(hash)){
			return;
		}
	}

	{
		LOCK_GUARD(m->mutex_album_list);
		m->hash_album_list.push_back(HashAlbumPair(hash, album));
	}

}

AlbumCoverFetchThread::HashLocationPair AlbumCoverFetchThread::take_current_location()
{
	if(m->location_list.count() > 0)
	{
		LOCK_GUARD(m->mutex_location_list);
		return m->location_list.takeLast();
	}

	else if(m->location_symlink_list.count() > 0)
	{
		LOCK_GUARD(m->mutex_symlink_list);
		return m->location_symlink_list.takeLast();
	}

	else if(m->location_online_list.count() > 0)
	{
		HashLocationPair p;
		{
			LOCK_GUARD(m->mutex_online_list)
			p = m->location_online_list.takeLast();
		}

		LOCK_GUARD(m->mutex_queued_hashes)
		m->queued_hashes << p.first;
		return p;
	}

	else {
		return AlbumCoverFetchThread::HashLocationPair();
	}
}


AlbumCoverFetchThread::Hash AlbumCoverFetchThread::get_hash(const Album& album)
{
	return album.name() + "-" + QString::number(album.id);
}


void AlbumCoverFetchThread::pause()
{
	m->paused = true;
	while(!m->in_paused_state){
		this->msleep(10);
	}
}

void AlbumCoverFetchThread::stop()
{
	m->stopped = true;
}

void AlbumCoverFetchThread::resume()
{
	::Util::sleep_ms(20);
	m->paused = false;
}

void AlbumCoverFetchThread::clear()
{
	{
		LOCK_GUARD(m->mutex_album_list)
		m->hash_album_list.clear();
	}

	{
		LOCK_GUARD(m->mutex_location_list)
		m->location_list.clear();
	}

	{
		LOCK_GUARD(m->mutex_online_list)
		m->location_online_list.clear();
	}

	{
		LOCK_GUARD(m->mutex_symlink_list)
		m->location_symlink_list.clear();
	}
}

void AlbumCoverFetchThread::done(const AlbumCoverFetchThread::Hash& hash)
{
	LOCK_GUARD(m->mutex_queued_hashes)
	m->queued_hashes.removeAll(hash);
}


