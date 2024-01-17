/* AlbumCoverFetchThread.cpp */

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

#include "AlbumCoverFetchThread.h"

#include "Utils/MetaData/Album.h"
#include "Components/Covers/CoverLocation.h"

#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/RandomGenerator.h"

#include <mutex>

using Cover::Location;
using Library::AlbumCoverFetchThread;
using Hash = AlbumCoverFetchThread::Hash;
using LockGuard = std::lock_guard<std::mutex>;

namespace
{
	constexpr const int MaxThreads = 20;

	std::mutex mutexAlbumList;
	std::mutex mutexQueuedHashes;
	std::mutex mutexHashLocationPairs;
}

struct AlbumCoverFetchThread::Private
{
	AlbumCoverFetchThread::HashAlbumList hashAlbumList;
	QList<HashLocationPair> hashLocationPairs;

	QStringList queuedHashes;

	std::atomic<int> timeToWait {0};
	std::atomic<bool> stopped {false};

	void pause(int ms = 10)
	{
		timeToWait = std::min<int>(timeToWait + ms, 70);
	}

	void wait()
	{
		const auto ms = std::min<int>(20, timeToWait);
		Util::sleepMs(static_cast<uint64_t>(ms));
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
	while(!m->stopped)
	{
		if(!m->mayRun())
		{
			continue;
		}

		int count;
		{
			[[maybe_unused]] const auto lockGuard = LockGuard(mutexAlbumList);
			count = m->hashAlbumList.count();
		}

		if(count == 0)
		{
			m->pause();
			continue;
		}

		while(true)
		{
			HashAlbumPair hashAlbumPair;
			{
				[[maybe_unused]] const auto lockGuard = LockGuard(mutexAlbumList);
				count = m->hashAlbumList.count();
				if(count == 0)
				{
					break;
				}

				hashAlbumPair = m->hashAlbumList.takeLast();
			}

			const auto& [hash, album] = hashAlbumPair;
			const auto location = Cover::Location::coverLocation(album);
			{
				[[maybe_unused]] const auto lockGuard = LockGuard(mutexHashLocationPairs);
				m->hashLocationPairs << HashLocationPair(hash, location);
			}

			emit sigNext();
		}
	}
}

void AlbumCoverFetchThread::addAlbum(const Album& album)
{
	if(m->stopped)
	{
		return;
	}

	m->pause();

	const auto hash = getHash(album);
	if(checkAlbum(hash))
	{
		spLog(Log::Develop, this) << "Already processing " << hash;
		return;
	}

	[[maybe_unused]] const auto lockGuard = LockGuard(mutexAlbumList);
	m->hashAlbumList.push_front(HashAlbumPair(hash, album));
	Util::Algorithm::shuffle(m->hashAlbumList);
}

bool AlbumCoverFetchThread::checkAlbum(const QString& hash)
{
	bool hasHash;
	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexHashLocationPairs);
		hasHash = Util::Algorithm::contains(m->hashLocationPairs, [&](const auto& hashLocationPair) {
			return (hashLocationPair.first == hash);
		});
	}

	if(hasHash)
	{
		spLog(Log::Crazy, this) << "Cover " << hash << " already in lookups";
		emit sigNext();
		return true;
	}

	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexQueuedHashes);
		if(m->queuedHashes.contains(hash))
		{
			spLog(Log::Crazy, this) << "Cover " << hash << " already in ready hashes";
			return true;
		}
	}

	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexAlbumList);
		hasHash = Util::Algorithm::contains(m->hashAlbumList, [&](const auto& hashLocationPair) {
			return (hashLocationPair.first == hash);
		});
	}

	if(hasHash)
	{
		spLog(Log::Crazy, this) << "Cover " << hash << " already in hash_album_list";
	}

	return hasHash;
}

AlbumCoverFetchThread::HashLocationPair AlbumCoverFetchThread::takeCurrentLookup()
{
	HashLocationPair hashLocationPair;

	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexHashLocationPairs);
		if(!m->hashLocationPairs.isEmpty())
		{
			hashLocationPair = m->hashLocationPairs.takeLast();
		}
	}

	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexQueuedHashes);
		m->queuedHashes.push_back(hashLocationPair.first);
	}

	return hashLocationPair;
}

void AlbumCoverFetchThread::removeHash(const AlbumCoverFetchThread::Hash& hash)
{
	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexQueuedHashes);
		m->queuedHashes.removeAll(hash);
	}

	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexHashLocationPairs);
		Util::Algorithm::removeIf(m->hashLocationPairs, [&](const auto& hashLocationPair) {
			return (hashLocationPair.first == hash);
		});
	}
}

AlbumCoverFetchThread::Hash AlbumCoverFetchThread::getHash(const Album& album)
{
	return QString("%1-%2")
		.arg(album.name())
		.arg(album.id());
}

void AlbumCoverFetchThread::stop()
{
	m->stopped = true;
}

void AlbumCoverFetchThread::clear()
{
	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexAlbumList);
		m->hashAlbumList.clear();
	}

	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexHashLocationPairs);
		m->hashLocationPairs.clear();
	}
}
