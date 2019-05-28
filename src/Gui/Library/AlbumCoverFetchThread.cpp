/* AlbumCoverFetchThread.cpp */

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

#include "AlbumCoverFetchThread.h"
#include "Utils/MetaData/Album.h"
#include "Components/Covers/CoverLookup.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookup.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/FileUtils.h"

#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include "Utils/Set.h"

#include <atomic>
#include <mutex>

using Cover::Location;
using Cover::Lookup;
using Hash=AlbumCoverFetchThread::Hash;
using AtomicBool=std::atomic<bool>;
using AtomicInt=std::atomic<int>;

namespace FileUtils=::Util::File;
static const int MaxThreads=20;

struct AlbumCoverFetchThread::Private
{
	AlbumCoverFetchThread::HashAlbumList hash_album_list;
	QList<HashLocationPair> lookups;

	QStringList			queued_hashes;

	std::mutex mutex_album_list;
	std::mutex mutex_queued_hashes;
	std::mutex mutex_lookup;

	AtomicInt	paused_to_go;
	AtomicInt	done;
	AtomicBool	stopped;
	AtomicBool	in_paused_state;

	Private()
	{
		init();
	}

	void init()
	{
		done = 0;
		stopped = false;
		hash_album_list.clear();
		in_paused_state = false;
		paused_to_go = 0;
	}

	void pause(int ms = 10)
	{
		paused_to_go = std::min<int>(paused_to_go + ms, 70);
	}

	void wait()
	{
		auto ms = std::min<int>(20, paused_to_go);
		Util::sleep_ms(ms);
		paused_to_go -= ms;
	}

	bool may_run()
	{
		if(stopped){
			in_paused_state = true;
			return false;
		}

		if(queued_hashes.count() >= MaxThreads) {
			in_paused_state = true;
			wait();
		};

		if(paused_to_go > 0) {
			in_paused_state = true;
			wait();
		}

		else {
			in_paused_state = false;
			return true;
		}

		return false;
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
	m->init();

	while(!m->stopped)
	{
		if(!m->may_run()){
			continue;
		}

		QList<HashAlbumPair> haps;

		{
			LOCK_GUARD(m->mutex_album_list)
			haps = m->hash_album_list;
		}
		
		if(haps.isEmpty())
		{
			m->pause();
			continue;	
		}

		for(const HashAlbumPair& hap : haps)
		{
			QString hash = hap.first;
			Album album = hap.second;
			Cover::Location cl = Cover::Location::xcover_location(album);
			{
				LOCK_GUARD(m->mutex_lookup);
				m->lookups << HashLocationPair(hash, cl);
			}

			emit sig_next();
		}

		{
			LOCK_GUARD(m->mutex_album_list)
			m->hash_album_list.clear();
		}
	}
}

void AlbumCoverFetchThread::add_album(const Album& album)
{
	if(m->stopped){
		sp_log(Log::Develop, this) << "Currently inactive";
		return;
	}

	m->pause();

	QString hash = get_hash(album);
	if(check_album(hash)){
		sp_log(Log::Develop, this) << "Already processing " << hash;
		return;
	}

	LOCK_GUARD(m->mutex_album_list)
	m->hash_album_list.push_front(HashAlbumPair(hash, album));
}

bool AlbumCoverFetchThread::check_album(const QString& hash)
{
	bool has_hash = false;
	{
		LOCK_GUARD(m->mutex_lookup)
		has_hash = ::Util::contains(m->lookups, [hash](const HashLocationPair& p){
			return (p.first == hash);
		});
	}

	if(has_hash){
		sp_log(Log::Crazy, this) << "Cover " << hash << " already in lookups";
		emit sig_next();
		return true;
	}

	{
		LOCK_GUARD(m->mutex_queued_hashes)
		if(m->queued_hashes.contains(hash)){
			sp_log(Log::Crazy, this) << "Cover " << hash << " already in queued hashes";
			return true;
		}
	}

	{
		LOCK_GUARD(m->mutex_album_list)
		has_hash = ::Util::contains(m->hash_album_list, [hash](const HashAlbumPair& p){
			return (p.first == hash);
		});
	}

	if(has_hash){
		sp_log(Log::Crazy, this) << "Cover " << hash << " already in hash_album_list";
	}

	return has_hash;
}

int AlbumCoverFetchThread::lookups_ready() const
{
	return m->lookups.size();
}

int AlbumCoverFetchThread::queued_hashes() const
{
	return m->queued_hashes.size();
}

int AlbumCoverFetchThread::unprocessed_hashes() const
{
	return m->hash_album_list.size();
}

AlbumCoverFetchThread::HashLocationPair AlbumCoverFetchThread::take_current_lookup()
{
	HashLocationPair ret;

	{
		LOCK_GUARD(m->mutex_lookup)
		if(!m->lookups.isEmpty()){
			ret = m->lookups.takeLast();
		}
	}

	{
		LOCK_GUARD(m->mutex_queued_hashes)
		m->queued_hashes.push_back(ret.first);
	}

	return ret;

}

void AlbumCoverFetchThread::done(const AlbumCoverFetchThread::Hash& hash)
{
	{
		LOCK_GUARD(m->mutex_queued_hashes)
		m->queued_hashes.removeAll(hash);
	}

	{
		LOCK_GUARD(m->mutex_lookup)
		for(int i=m->lookups.size() - 1; i>=0; i--)
		{
			if(m->lookups[i].first == hash){
				m->lookups.removeAt(i);
			}
		}
	}
}

AlbumCoverFetchThread::Hash AlbumCoverFetchThread::get_hash(const Album& album)
{
	return album.name() + "-" + QString::number(album.id);
}

void AlbumCoverFetchThread::pause()
{
	m->pause();
}

void AlbumCoverFetchThread::stop()
{
	m->stopped = true;
}

void AlbumCoverFetchThread::resume()
{
	m->paused_to_go = 0;
}

void AlbumCoverFetchThread::clear()
{
	{
		LOCK_GUARD(m->mutex_album_list)
		m->hash_album_list.clear();
	}

	{
		LOCK_GUARD(m->mutex_lookup)
		m->lookups.clear();
	}
}


