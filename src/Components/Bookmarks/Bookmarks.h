/* Bookmarks.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include "BookmarksBase.h"
#include "Components/PlayManager/PlayState.h"

#include <QList>

class Bookmark;
class MetaData;

/**
 * @brief The Bookmarks logic class
 * @ingroup Bookmarks
 */
class Bookmarks :
		public BookmarksBase
{
	Q_OBJECT
	PIMPL(Bookmarks)

signals:
	/**
	 * @brief emitted when bookmarks have changed
	 * @param bookmarks new bookmarks
	 */
	void sig_bookmarks_changed();

	/**
	 * @brief previous bookmark has changed
	 * @param bm new bookmark. Check for Bookmark::is_valid()
	 */
	void sig_prev_changed(const Bookmark& bm);

	/**
	 * @brief next bookmark has changed
	 * @param bm new bookmark. Check for Bookmark::is_valid()
	 */
	void sig_next_changed(const Bookmark& bm);

public:

	explicit Bookmarks(QObject *parent);
	~Bookmarks();


	/**
	 * @brief Jump to specific bookmark
	 * @param idx bookmark index
	 * @return true if index was valid
	 */
	bool jump_to(int idx);

	/**
	 * @brief Jump to next bookmark
	 * @return true if successful, false else
	 */
	bool jump_next();

	/**
	 * @brief Jump to previous bookmark
	 * @return true if successful, false else
	 */
	bool jump_prev();


	/**
	 * @brief tries to set the loop between the current two indices
	 * @param b switch loop on or off
	 * @return false if the two current indices are invalid or if b == false. True else
	 */
	bool set_loop(bool b);

	BookmarksBase::CreationStatus create();


	bool remove(int idx) override;


private slots:
	/**
	 * @brief track position has changed
	 * @param pos new position in ms
	 */
	void pos_changed_ms(MilliSeconds pos);

	/**
	 * @brief current track has changed
	 * @param md new MetaData object
	 */
	void track_changed(const MetaData& md);

	/**
	 * @brief current playstate has changed
	 * @param state new playstate
	 */
	void playstate_changed(PlayState state);


private:

	using BookmarksBase::create;
	using BookmarksBase::set_metadata;

	/**
	 * @brief fetch bookmarks from db and emit sig_bookmarks_changed signal
	 */
	bool load() override;
};

#endif // BOOKMARKS_H
