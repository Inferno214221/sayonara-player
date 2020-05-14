/* GUI_SoundCloudLibrary.h */

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

#ifndef GUI_SOUNDCLOUDLIBRARY_H
#define GUI_SOUNDCLOUDLIBRARY_H

#include "Gui/Library/GUI_AbstractLibrary.h"
#include "Utils/Pimpl.h"

class QFrame;

UI_FWD(GUI_SoundcloudLibrary)

namespace SC
{
	class Library;
	class GUI_ArtistSearch;

	class GUI_Library :
			public ::Library::GUI_AbstractLibrary
	{
		Q_OBJECT
		UI_CLASS(GUI_SoundcloudLibrary)
		PIMPL(GUI_Library)

		public:
			explicit GUI_Library(SC::Library* library, QWidget* parent=nullptr);
			~GUI_Library() override;

			QMenu*		getMenu() const;
			QFrame*		headerFrame() const;

			QList<::Library::Filter::Mode> searchOptions() const override;

		private slots:
			void btnAddClicked();

		protected:
			void languageChanged() override;

			::Library::TrackDeletionMode showDeleteDialog(int n_tracks) override;

			::Library::TableView* lvArtist() const override;
			::Library::TableView* lvAlbum() const override;
			::Library::TableView* lvTracks() const override;

			::Library::SearchBar* leSearch() const override;

			void showEvent(QShowEvent* e) override;
	};
}
#endif // GUI_SOUNDCLOUDLIBRARY_H
