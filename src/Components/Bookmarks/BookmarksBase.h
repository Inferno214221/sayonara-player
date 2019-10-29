/* BookmarksBase.h */

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

#ifndef BOOKMARKSBASE_H
#define BOOKMARKSBASE_H

#include "Utils/Pimpl.h"
#include "Bookmark.h"
#include "Database/ConnectorProvider.h"

#include <QObject>

class MetaData;
class BookmarksBase :
		public QObject,
		public DB::ConnectorConsumer
{
	Q_OBJECT
	PIMPL(BookmarksBase)

public:
	enum class CreationStatus : unsigned char
	{
		Success,
		AlreadyThere,
		NoDBTrack,
		DBError,
		OtherError
	};

	explicit BookmarksBase(QObject* parent);
	virtual ~BookmarksBase();

	void setup_databases() override;

	/**
	 * @brief create a new bookmark for current track and current position
	 * @return true if successful, else false
	 */
	virtual CreationStatus create(Seconds timestamp);

	virtual bool load();

	/**
	 * @brief remove single bookmark from database for current track
	 * @param idx index
	 * @return
	 */
	virtual bool remove(int idx);

	/**
	 * @brief get the current track
	 * @return
	 */
	MetaData metadata() const;
	void set_metadata(const MetaData& md);

	const QList<Bookmark> bookmarks() const;
	void set_bookmarks(const QList<Bookmark> bookmarks);

	int count();
	void add(const Bookmark& bookmark);
	void clear();

	const Bookmark& bookmark(int idx) const;
	Bookmark& bookmark(int idx);

	void sort();
};

#endif // BOOKMARKSBASE_H
