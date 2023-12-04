/* Bookmarks.h */

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

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include "BookmarkStorage.h"

#include <QList>
#include <QObject>

class Bookmark;
class MetaData;
class PlayManager;

class Bookmarks :
	public QObject
{
	Q_OBJECT
	PIMPL(Bookmarks)

	signals:
		void sigBookmarksChanged();
		void sigPreviousChanged(const Bookmark& bm);
		void sigNextChanged(const Bookmark& bm);

	public:
		explicit Bookmarks(PlayManager* playManager, QObject* parent = nullptr);
		~Bookmarks() override;

		bool jumpTo(int idx);
		bool jumpNext();
		bool jumpPrevious();
		bool setLoop(bool b);

		[[nodiscard]] int count() const;

		BookmarkStorage::CreationStatus create();

		bool remove(int index);
		[[nodiscard]] const QList<Bookmark>& bookmarks() const;

		[[nodiscard]] const MetaData& currentTrack() const;

	private slots:
		void positionChangedMs(MilliSeconds positionMs);
		void currentTrackChanged(const MetaData& track);
		void playstateChanged(PlayState state);
};

#endif // BOOKMARKS_H
