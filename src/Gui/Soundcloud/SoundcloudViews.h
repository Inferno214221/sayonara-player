/* SoundcloudViews.h */

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


#ifndef SOUNDCLOUDVIEWS_H
#define SOUNDCLOUDVIEWS_H

#include "Gui/Library/TableView/TrackView.h"
#include "Gui/Library/TableView/AlbumView.h"
#include "Gui/Library/TableView/ArtistView.h"

namespace SC
{
	class TrackView : public ::Library::TrackView
	{
		Q_OBJECT
		public:
			using ::Library::TrackView::TrackView;
			::Library::ContextMenu::Entries contextMenuEntries() const override;
	};

	class AlbumView : public ::Library::AlbumView
	{
		Q_OBJECT
		public:
			using ::Library::AlbumView::AlbumView;
			::Library::ContextMenu::Entries contextMenuEntries() const override;
	};

	class ArtistView : public ::Library::ArtistView
	{
		Q_OBJECT
		signals:
			void sigAddArtistTriggered();

		public:
			using ::Library::ArtistView::ArtistView;
			::Library::ContextMenu::Entries contextMenuEntries() const override;

		// ItemView interface
		protected:
			void initContextMenu() override;
	};
}

#endif // SOUNDCLOUDVIEWS_H
