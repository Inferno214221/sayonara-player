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

#include "Bookmark.h"
#include "Bookmarks.h"

#include "Utils/globals.h"
#include "Utils/MetaData/MetaData.h"

#include "Components/PlayManager/PlayManager.h"

struct Bookmarks::Private
{
	PlayManager*	playManager=nullptr;

	int				previousIndex;
	int				nextIndex;

	Seconds			timeOffset;
	Seconds			currentTime;

	Seconds			loopStart;
	Seconds			loopEnd;

	Private() :
		playManager(PlayManagerProvider::instance()->playManager()),
	    previousIndex(-1),
	    nextIndex(-1),
	    timeOffset(0),
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
        timeOffset = 0;
		loopStart = 0;
		loopEnd = 0;
        currentTime = 0;
	}
};


Bookmarks::Bookmarks(QObject* parent) :
	BookmarksBase(parent)
{
	m = Pimpl::make<Bookmarks::Private>();

	connect(m->playManager, &PlayManager::sigCurrentTrackChanged, this, &Bookmarks::currentTrackChanged);
	connect(m->playManager, &PlayManager::sigPositionChangedMs, this, &Bookmarks::positionChangedMs);
	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &Bookmarks::playstateChanged);

	setMetadata(m->playManager->currentTrack());
}

Bookmarks::~Bookmarks() = default;

bool Bookmarks::load()
{
	bool success = BookmarksBase::load();
	if(success){
		emit sigBookmarksChanged();
	}

	return success;
}

BookmarksBase::CreationStatus Bookmarks::create()
{
	BookmarksBase::CreationStatus status = BookmarksBase::create(m->playManager->currentPositionMs() / 1000);
	if(status == BookmarksBase::CreationStatus::Success)
	{
		emit sigBookmarksChanged();
	}

	return status;
}


bool Bookmarks::remove(int idx)
{
	bool success = BookmarksBase::remove(idx);
	if(success){
		emit sigBookmarksChanged();
	}

	return success;
}


bool Bookmarks::jumpTo(int idx)
{
	if(!Util::between(idx, this->count()) )
	{
		return false;
	}

	if(idx < 0){
		m->playManager->seekAbsoluteMs(0);
	}
	else
	{
		MilliSeconds new_time = bookmark(idx).timestamp() * 1000;
		m->playManager->seekAbsoluteMs(new_time);
	}

	return true;
}


bool Bookmarks::jumpNext()
{
	if( !Util::between(m->nextIndex, this->count()) )
	{
		emit sigNextChanged(Bookmark());
		return false;
	}

	jumpTo(m->nextIndex);

	return true;
}


bool Bookmarks::jumpPrevious()
{
	if( m->previousIndex >= this->count() )
	{
		emit sigPreviousChanged(Bookmark());
		return false;
	}

	jumpTo(m->previousIndex);

	return true;
}


void Bookmarks::positionChangedMs(MilliSeconds pos_ms)
{
	m->currentTime = (Seconds) (pos_ms / 1000);

    if (m->currentTime >= m->loopEnd)
    {
        if (m->loopEnd != 0)
        {
            jumpPrevious();
            return;
        }
    }

    if(this->count() == 0){
		return;
	}

	m->previousIndex=-1;
	m->nextIndex=-1;

	const QList<Bookmark> bookmarks = this->bookmarks();
	for(auto it=bookmarks.begin(); it != bookmarks.end(); it++)
	{
		Seconds time = it->timestamp();

		if(time + m->timeOffset < m->currentTime)
		{
			m->previousIndex = std::distance(bookmarks.begin(), it);
		}

		else if(time > m->currentTime)
		{
			if(m->nextIndex == -1){
				m->nextIndex = std::distance(bookmarks.begin(), it);
				break;
			}
		}
	}

	if( Util::between(m->previousIndex, this->count()) ){
		emit sigPreviousChanged(this->bookmark(m->previousIndex));
	}
	else{
		emit sigPreviousChanged(Bookmark());
	}

	if( Util::between(m->nextIndex, this->count()) ){
		emit sigNextChanged(this->bookmark(m->nextIndex));
	}
	else{
		emit sigNextChanged(Bookmark());
	}
}


void Bookmarks::currentTrackChanged(const MetaData& md)
{
	BookmarksBase::setMetadata(md);

	emit sigBookmarksChanged();
	emit sigPreviousChanged(Bookmark());
	emit sigNextChanged(Bookmark());
}


void Bookmarks::playstateChanged(PlayState state)
{
	if(state == PlayState::Stopped)
	{
		m->reset();

		emit sigBookmarksChanged();
		emit sigPreviousChanged(Bookmark());
		emit sigNextChanged(Bookmark());
	}
}

bool Bookmarks::setLoop(bool b)
{
	bool ret = false;

	m->loopStart = 0;
	m->loopEnd = 0;

	if(b)
	{
		if( Util::between(m->previousIndex, this->count()) &&
			Util::between(m->nextIndex, this->count()) )
		{
			m->loopStart = bookmark(m->previousIndex).timestamp();
			m->loopEnd = bookmark(m->nextIndex).timestamp();
			ret = true;
		}
	}

	return ret;
}
