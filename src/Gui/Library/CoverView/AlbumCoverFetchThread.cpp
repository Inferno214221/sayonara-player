/* AlbumCoverFetchThread.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include <random>
#include "Utils/MetaData/Album.h"
#include "Components/Covers/CoverLocation.h"

#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Mutex.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

using Cover::Location;
using Library::AlbumCoverFetchThread;
using Hash = AlbumCoverFetchThread::Hash;
using AtomicBool = std::atomic<bool>;
using AtomicInt = std::atomic<int>;

namespace Algorithm = Util::Algorithm;

static const int MaxThreads = 20;

struct AlbumCoverFetchThread::Private
{
	AlbumCoverFetchThread::HashAlbumList hashAlbumList;
	QList<HashLocationPair> hashLocationPairs;

	QStringList queuedHashes;

	std::mutex mutexAlbumList;
	std::mutex mutexQueuedHashes;
	std::mutex mutexHashLocationPairs;

	AtomicInt timeToWait;
	AtomicBool stopped;

	Private() :
		timeToWait(0),
		stopped(false)
	{
		init();
	}

	void init()
	{
		stopped = false;
		hashAlbumList.clear();
		timeToWait = 0;
	}

	void pause(int ms = 10)
	{
		timeToWait = std::min<int>(timeToWait + ms, 70);
	}

	void wait()
	{
		auto ms = std::min<int>(20, timeToWait);
		Util::sleepMs(uint64_t(ms));
		timeToWait -= ms;
	}

	bool mayRun()
	{
		if(stopped)
		{
			return false;
		}

		if(queuedHashes.count() >= MaxThreads)
		{
			wait();
		}

		if(timeToWait > 0)
		{
			wait();
			return false;
		}

		return true;
	}
};

AlbumCoverFetchThread::AlbumCoverFetchThread(QObject* parent) :
	QThread(parent)
{
	m = Pimpl::make<Private>();
}

AlbumCoverFetchThread::~AlbumCoverFetchThread() = default;

void AlbumCoverFetchThread::run()
{
	m->init();

	while(!m->stopped)
	{
		if(!m->mayRun())
		{
			continue;
		}

		int count;
		{
			LOCK_GUARD(m->mutexAlbumList)
			count = m->hashAlbumList.count();
		}

		if(count == 0)
		{
			m->pause();
			continue;
		}

		while(true)
		{
			HashAlbumPair p;
			{
				LOCK_GUARD(m->mutexAlbumList)
				count = m->hashAlbumList.count();
				if(count == 0)
				{
					break;
				}

				p = m->hashAlbumList.takeLast();
			}

			Hash hash = p.first;
			Album album = p.second;

			Cover::Location cl = Cover::Location::xcoverLocation(album);
			{
				LOCK_GUARD(m->mutexHashLocationPairs)
				m->hashLocationPairs << HashLocationPair(hash, cl);
			}

			emit sigNext();
			Util::sleepMs(25);
		}
	}
}

void AlbumCoverFetchThread::addAlbum(const Album& album)
{
	if(m->stopped)
	{
		spLog(Log::Develop, this) << "Currently inactive";
		return;
	}

	m->pause();

	const QString hash = getHash(album);
	if(checkAlbum(hash))
	{
		spLog(Log::Develop, this) << "Already processing " << hash;
		return;
	}

	LOCK_GUARD(m->mutexAlbumList)
	m->hashAlbumList.push_front(HashAlbumPair(hash, album));
	std::shuffle(m->hashAlbumList.begin(), m->hashAlbumList.end(), std::mt19937(std::random_device()()));
}

bool AlbumCoverFetchThread::checkAlbum(const QString& hash)
{
	bool hasHash;
	{
		LOCK_GUARD(m->mutexHashLocationPairs)
		hasHash = Algorithm::contains(m->hashLocationPairs, [hash](const HashLocationPair& p) {
			return (p.first == hash);
		});
	}

	if(hasHash)
	{
		spLog(Log::Crazy, this) << "Cover " << hash << " already in lookups";
		emit sigNext();
		return true;
	}

	{
		LOCK_GUARD(m->mutexQueuedHashes)
		if(m->queuedHashes.contains(hash))
		{
			spLog(Log::Crazy, this) << "Cover " << hash << " already in queued hashes";
			return true;
		}
	}

	{
		LOCK_GUARD(m->mutexAlbumList)
		hasHash = Algorithm::contains(m->hashAlbumList, [hash](const HashAlbumPair& p) {
			return (p.first == hash);
		});
	}

	if(hasHash)
	{
		spLog(Log::Crazy, this) << "Cover " << hash << " already in hash_album_list";
	}

	return hasHash;
}

int AlbumCoverFetchThread::lookupsReady() const
{
	return m->hashLocationPairs.size();
}

int AlbumCoverFetchThread::queuedHashes() const
{
	return m->queuedHashes.size();
}

int AlbumCoverFetchThread::unprocessedHashes() const
{
	return m->hashAlbumList.size();
}

AlbumCoverFetchThread::HashLocationPair AlbumCoverFetchThread::takeCurrentLookup()
{
	HashLocationPair ret;

	{
		LOCK_GUARD(m->mutexHashLocationPairs)
		if(!m->hashLocationPairs.isEmpty())
		{
			ret = m->hashLocationPairs.takeLast();
		}
	}

	{
		LOCK_GUARD(m->mutexQueuedHashes)
		m->queuedHashes.push_back(ret.first);
	}

	return ret;

}

void AlbumCoverFetchThread::removeHash(const AlbumCoverFetchThread::Hash& hash)
{
	{
		LOCK_GUARD(m->mutexQueuedHashes)
		m->queuedHashes.removeAll(hash);
	}

	{
		LOCK_GUARD(m->mutexHashLocationPairs)
		for(int i = m->hashLocationPairs.size() - 1; i >= 0; i--)
		{
			if(m->hashLocationPairs[i].first == hash)
			{
				m->hashLocationPairs.removeAt(i);
			}
		}
	}
}

AlbumCoverFetchThread::Hash AlbumCoverFetchThread::getHash(const Album& album)
{
	return album.name() + "-" + QString::number(album.id());
}

void AlbumCoverFetchThread::stop()
{
	m->stopped = true;
}

void AlbumCoverFetchThread::clear()
{
	{
		LOCK_GUARD(m->mutexAlbumList)
		m->hashAlbumList.clear();
	}

	{
		LOCK_GUARD(m->mutexHashLocationPairs)
		m->hashLocationPairs.clear();
	}
}
