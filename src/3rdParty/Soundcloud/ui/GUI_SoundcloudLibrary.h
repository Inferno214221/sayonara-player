/* GUI_SoundCloudLibrary.h */

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

#ifndef GUI_SOUNDCLOUDLIBRARY_H
#define GUI_SOUNDCLOUDLIBRARY_H

#include "GUI/Library/GUI_AbstractLibrary.h"
#include "Utils/Pimpl.h"

#include "3rdParty/Soundcloud/ui/GUI_SoundcloudArtistSearch.h"

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
		explicit GUI_Library(SC::Library* library, QWidget *parent=nullptr);
		~GUI_Library();

		QMenu*		get_menu() const;
		QFrame*		header_frame() const;

		QList<::Library::Filter::Mode> search_options() const override;

	protected:
		::Library::TrackDeletionMode show_delete_dialog(int n_tracks) override;

		::Library::TableView* lv_artist() const override;
		::Library::TableView* lv_album() const override;
		::Library::TableView* lv_tracks() const override;

		::Library::SearchBar* le_search() const override;

		void showEvent(QShowEvent *e) override;

	protected slots:
		void btn_add_clicked();
	};
}
#endif // GUI_SOUNDCLOUDLIBRARY_H
