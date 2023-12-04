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

#include "Components/PlayManager/PlayManager.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/globals.h"

namespace
{
	constexpr const Seconds TimeOffset = 1;
}

struct Bookmarks::Private
{
	PlayManager* playManager;
	BookmarkStoragePtr bookmarkStorage;

	int previousIndex {-1};
	int nextIndex {-1};
	Seconds currentTime {0};
	Seconds loopEnd {0};

	explicit Private(PlayManager* playManager) :
		playManager(playManager),
		bookmarkStorage(BookmarkStorage::create(playManager->currentTrack()))
	{
		reset();
	}

	void reset()
	{
		previousIndex = -1;
		nextIndex = -1;
		currentTime = 0;
		loopEnd = 0;
	}
};

Bookmarks::Bookmarks(PlayManager* playManager, QObject* parent) :
	QObject(parent),
	m {Pimpl::make<Bookmarks::Private>(playManager)}
{
	connect(m->playManager, &PlayManager::sigCurrentTrackChanged, this, &Bookmarks::currentTrackChanged);
	connect(m->playManager, &PlayManager::sigPositionChangedMs, this, &Bookmarks::positionChangedMs);
	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &Bookmarks::playstateChanged);
}

Bookmarks::~Bookmarks() = default;

BookmarkStorage::CreationStatus Bookmarks::create()
{
	const auto status = m->bookmarkStorage->create(m->playManager->currentPositionMs() / 1000);
	if(status == BookmarkStorage::CreationStatus::Success)
	{
		emit sigBookmarksChanged();
	}

	return status;
}

bool Bookmarks::remove(const int index)
{
	const auto removed = m->bookmarkStorage->remove(index);
	if(removed)
	{
		emit sigBookmarksChanged();
	}

	return removed;
}

const QList<Bookmark>& Bookmarks::bookmarks() const { return m->bookmarkStorage->bookmarks(); }

int Bookmarks::count() const { return m->bookmarkStorage->count(); }

bool Bookmarks::jumpTo(const int idx)
{
	const auto isIndexValid = Util::between(idx, count());
	if(isIndexValid)
	{
		const auto newTime = (idx >= 0)
		                     ? static_cast<MilliSeconds>(m->bookmarkStorage->bookmark(idx).timestamp() * 1000)
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

void Bookmarks::positionChangedMs(const MilliSeconds positionMs)
{
	m->currentTime = static_cast<Seconds>(positionMs / 1000); // NOLINT(readability-magic-numbers)
	if(m->loopEnd != 0 && m->currentTime >= m->loopEnd)
	{
		jumpPrevious();
		return;
	}

	if(count() == 0)
	{
		return;
	}

	const auto& bookmarks = m->bookmarkStorage->bookmarks();

	const auto it = std::find_if(bookmarks.crbegin(), bookmarks.crend(), [this](const Bookmark& bookmark) {
		return (bookmark.timestamp() < m->currentTime - TimeOffset);
	});

	m->previousIndex = (it != bookmarks.crend())
	                   ? static_cast<int>(std::distance(it, bookmarks.crend()) - 1)
	                   : -1;

	const auto previousBookmark = (m->previousIndex >= 0)
	                              ? m->bookmarkStorage->bookmark(m->previousIndex)
	                              : Bookmark {};

	m->nextIndex = Util::Algorithm::indexOf(bookmarks, [this](const auto& bookmark) {
		return (bookmark.timestamp() > m->currentTime);
	});

	const auto nextBookmark = (m->nextIndex >= 0)
	                          ? m->bookmarkStorage->bookmark(m->nextIndex)
	                          : Bookmark {};

	emit sigPreviousChanged(previousBookmark);
	emit sigNextChanged(nextBookmark);
}

void Bookmarks::currentTrackChanged(const MetaData& track)
{
	m->bookmarkStorage->setTrack(track);

	emit sigBookmarksChanged();
	emit sigPreviousChanged(Bookmark {});
	emit sigNextChanged(Bookmark {});
}

void Bookmarks::playstateChanged(const PlayState state)
{
	if(state == PlayState::Stopped)
	{
		m->reset();

		emit sigBookmarksChanged();
		emit sigPreviousChanged(Bookmark {});
		emit sigNextChanged(Bookmark {});
	}
}

bool Bookmarks::setLoop(const bool b)
{
	m->loopEnd = 0;

	if(!b)
	{
		return false;
	}

	const auto canLoop = Util::between(m->previousIndex, count()) &&
	                     Util::between(m->nextIndex, count());
	if(canLoop)
	{
		m->loopEnd = m->bookmarkStorage->bookmark(m->nextIndex).timestamp();
	}

	return canLoop;
}

const MetaData& Bookmarks::currentTrack() const { return m->bookmarkStorage->track(); }
