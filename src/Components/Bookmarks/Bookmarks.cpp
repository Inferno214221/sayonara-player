/* Bookmarks.cpp */

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

#include "Bookmark.h"
#include "Bookmarks.h"
#include "Utils/globals.h"
#include "Utils/MetaData/MetaData.h"

#include "Components/PlayManager/PlayManager.h"

struct Bookmarks::Private
{
	PlayManager*	play_manager=nullptr;

	int				prev_idx;
	int				next_idx;

	Seconds			time_offset;
	Seconds			cur_time;

	Seconds			loop_start;
	Seconds			loop_end;

	Private() :
		time_offset(1)
	{
		play_manager = PlayManager::instance();

		reset();
	}

	void reset()
	{
		cur_time = 0;
		prev_idx = -1;
		next_idx = -1;
		loop_start = 0;
		loop_end = 0;
	}
};


Bookmarks::Bookmarks(QObject *parent) :
	BookmarksBase(parent)
{
	m = Pimpl::make<Bookmarks::Private>();

	connect(m->play_manager, &PlayManager::sig_track_changed, this, &Bookmarks::track_changed);
	connect(m->play_manager, &PlayManager::sig_position_changed_ms, this, &Bookmarks::pos_changed_ms);
	connect(m->play_manager, &PlayManager::sig_playstate_changed, this, &Bookmarks::playstate_changed);

	set_metadata(m->play_manager->current_track());
}

Bookmarks::~Bookmarks() {}

bool Bookmarks::load()
{
	bool success = BookmarksBase::load();
	if(success){
		emit sig_bookmarks_changed();
	}

	return success;
}

BookmarksBase::CreationStatus Bookmarks::create()
{
	BookmarksBase::CreationStatus status = BookmarksBase::create(m->play_manager->current_position_ms() / 1000);
	if(status == BookmarksBase::CreationStatus::Success)
	{
		emit sig_bookmarks_changed();
	}

	return status;
}


bool Bookmarks::remove(int idx)
{
	bool success = BookmarksBase::remove(idx);
	if(success){
		emit sig_bookmarks_changed();
	}

	return success;
}


bool Bookmarks::jump_to(int idx)
{
	if(!between(idx, this->count()) )
	{
		return false;
	}

	if(idx < 0){
		m->play_manager->seek_abs_ms(0);
	}
	else
	{
		MilliSeconds new_time = bookmark(idx).timestamp() * 1000;
		m->play_manager->seek_abs_ms(new_time);
	}

	return true;
}


bool Bookmarks::jump_next()
{
	if( !between(m->next_idx, this->count()) )
	{
		emit sig_next_changed(Bookmark());
		return false;
	}

	jump_to(m->next_idx);

	return true;
}


bool Bookmarks::jump_prev()
{
	if( m->prev_idx >= this->count() )
	{
		emit sig_prev_changed(Bookmark());
		return false;
	}

	jump_to(m->prev_idx);

	return true;
}


void Bookmarks::pos_changed_ms(MilliSeconds pos_ms)
{
	m->cur_time = (Seconds) (pos_ms / 1000);

	if( m->cur_time >= m->loop_end &&
		m->loop_end != 0)
	{
		jump_prev();
		return;
	}

	if(this->count() == 0){
		return;
	}

	m->prev_idx=-1;
	m->next_idx=-1;

	const QList<Bookmark> bookmarks = this->bookmarks();
	for(auto it=bookmarks.begin(); it != bookmarks.end(); it++)
	{
		Seconds time = it->timestamp();

		if(time + m->time_offset < m->cur_time)
		{
			m->prev_idx = std::distance(bookmarks.begin(), it);
		}

		else if(time > m->cur_time)
		{
			if(m->next_idx == -1){
				m->next_idx = std::distance(bookmarks.begin(), it);
				break;
			}
		}
	}

	if( between(m->prev_idx, this->count()) ){
		emit sig_prev_changed(this->bookmark(m->prev_idx));
	}
	else{
		emit sig_prev_changed(Bookmark());
	}

	if( between(m->next_idx, this->count()) ){
		emit sig_next_changed(this->bookmark(m->next_idx));
	}
	else{
		emit sig_next_changed(Bookmark());
	}
}


void Bookmarks::track_changed(const MetaData& md)
{
	BookmarksBase::set_metadata(md);

	emit sig_bookmarks_changed();
	emit sig_prev_changed(Bookmark());
	emit sig_next_changed(Bookmark());
}


void Bookmarks::playstate_changed(PlayState state)
{
	if(state == PlayState::Stopped)
	{
		m->reset();

		emit sig_bookmarks_changed();
		emit sig_prev_changed(Bookmark());
		emit sig_next_changed(Bookmark());
	}
}

bool Bookmarks::set_loop(bool b)
{
	bool ret = false;

	m->loop_start = 0;
	m->loop_end = 0;

	if(b)
	{
		if( between(m->prev_idx, this->count()) &&
			between(m->next_idx, this->count()) )
		{
			m->loop_start = bookmark(m->prev_idx).timestamp();
			m->loop_end = bookmark(m->next_idx).timestamp();
			ret = true;
		}
	}

	return ret;
}
