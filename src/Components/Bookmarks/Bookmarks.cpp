/* Bookmarks.cpp */

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

#include <Utils/Logger/Logger.h>
#include "Bookmark.h"
#include "Bookmarks.h"
#include "BookmarkStorage.h"

#include "Utils/Algorithm.h"
#include "Utils/globals.h"
#include "Utils/MetaData/MetaData.h"

#include "Interfaces/PlayManager.h"

namespace
{
	static constexpr const Seconds TimeOffset = 1;
}

struct Bookmarks::Private
{
	PlayManager* playManager;
	BookmarkStorage bookmarkDbAccessor;

	int previousIndex;
	int nextIndex;

	Seconds currentTime;

	Seconds loopStart;
	Seconds loopEnd;

	Private(PlayManager* playManager) :
		playManager(playManager),
		bookmarkDbAccessor(BookmarkStorage{playManager->currentTrack()}),
		previousIndex(-1),
		nextIndex(-1),
		currentTime(0),
		loopStart(0),
		loopEnd(0)
	{
		reset();
	}

	void reset()
	{
		previousIndex = -1;
		nextIndex = -1;
		currentTime = 0;
		loopStart = 0;
		loopEnd = 0;
	}
};

Bookmarks::Bookmarks(PlayManager* playManager, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Bookmarks::Private>(playManager);

	connect(m->playManager, &PlayManager::sigCurrentTrackChanged, this, &Bookmarks::currentTrackChanged);
	connect(m->playManager, &PlayManager::sigPositionChangedMs, this, &Bookmarks::positionChangedMs);
	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &Bookmarks::playstateChanged);
}

Bookmarks::~Bookmarks() = default;

BookmarkStorage::CreationStatus Bookmarks::create()
{
	const auto status = m->bookmarkDbAccessor.create(m->playManager->currentPositionMs() / 1000);
	if(status == BookmarkStorage::CreationStatus::Success)
	{
		emit sigBookmarksChanged();
	}

	return status;
}

bool Bookmarks::remove(int index)
{
	const auto removed = m->bookmarkDbAccessor.remove(index);
	if(removed)
	{
		emit sigBookmarksChanged();
	}

	return removed;
}

const QList<Bookmark>& Bookmarks::bookmarks() const
{
	return m->bookmarkDbAccessor.bookmarks();
}

int Bookmarks::count() const
{
	return m->bookmarkDbAccessor.count();
}

bool Bookmarks::jumpTo(int idx)
{
	const auto isIndexValid = Util::between(idx, count());
	if(isIndexValid)
	{
		const auto newTime = (idx >= 0)
		                     ? static_cast<MilliSeconds>(m->bookmarkDbAccessor.bookmark(idx).timestamp() * 1000)
		                     : 0;

		m->playManager->seekAbsoluteMs(newTime);
	}

	return isIndexValid;
}

bool Bookmarks::jumpNext()
{
	if(!Util::between(m->nextIndex, count()))
	{
		emit sigNextChanged(Bookmark {});
		return false;
	}

	jumpTo(m->nextIndex);

	return true;
}

bool Bookmarks::jumpPrevious()
{
	if(m->previousIndex >= count())
	{
		emit sigPreviousChanged(Bookmark());
		return false;
	}

	jumpTo(m->previousIndex);

	return true;
}

void Bookmarks::positionChangedMs(MilliSeconds positionMs)
{
	m->currentTime = static_cast<Seconds>(positionMs / 1000);
	if(m->loopEnd != 0 && m->currentTime >= m->loopEnd)
	{
		jumpPrevious();
		return;
	}

	if(this->count() == 0)
	{
		return;
	}

	const auto& bookmarks = m->bookmarkDbAccessor.bookmarks();

	const auto it = std::find_if(bookmarks.crbegin(), bookmarks.crend(), [&](const Bookmark& bookmark){
		return (bookmark.timestamp() < m->currentTime - TimeOffset);
	});

	m->previousIndex = (it != bookmarks.crend())
		? std::distance(it, bookmarks.crend()) - 1
		: -1;

	const auto previousBookmark = (m->previousIndex >= 0)
	                              ? m->bookmarkDbAccessor.bookmark(m->previousIndex)
	                              : Bookmark {};

	m->nextIndex = Util::Algorithm::indexOf(bookmarks, [&](const auto& bookmark){
		return (bookmark.timestamp() > m->currentTime);
	});

	const auto nextBookmark = (m->nextIndex >= 0)
	                          ? m->bookmarkDbAccessor.bookmark(m->nextIndex)
	                          : Bookmark {};

	emit sigPreviousChanged(previousBookmark);
	emit sigNextChanged(nextBookmark);
}

void Bookmarks::currentTrackChanged(const MetaData& track)
{
	m->bookmarkDbAccessor.setTrack(track);

	emit sigBookmarksChanged();
	emit sigPreviousChanged(Bookmark {});
	emit sigNextChanged(Bookmark {});
}

void Bookmarks::playstateChanged(PlayState state)
{
	if(state == PlayState::Stopped)
	{
		m->reset();

		emit sigBookmarksChanged();
		emit sigPreviousChanged(Bookmark{});
		emit sigNextChanged(Bookmark{});
	}
}

bool Bookmarks::setLoop(bool b)
{
	m->loopStart = 0;
	m->loopEnd = 0;

	if(!b)
	{
		return false;
	}

	const auto canLoop = Util::between(m->previousIndex, count()) &&
	                     Util::between(m->nextIndex, count());
	if(canLoop)
	{
		m->loopStart = m->bookmarkDbAccessor.bookmark(m->previousIndex).timestamp();
		m->loopEnd = m->bookmarkDbAccessor.bookmark(m->nextIndex).timestamp();
	}

	return canLoop;
}

const MetaData& Bookmarks::currentTrack() const
{
	return m->bookmarkDbAccessor.track();
}
